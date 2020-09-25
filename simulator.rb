#!/usr/bin/env ruby

require 'json'
require 'net/http'
require 'observer'
require 'pathname'
require 'uri'

require 'rubyserial'
require_relative 'config'

class ProjectionScreen
  UP_TIME = 23
  DOWN_TIME = 21

  def initialize(ip, port)
    @socket = TCPSocket.new(ip, 4998)
    @port = port
  end

  def up_time
    UP_TIME
  end

  def down_time
    DOWN_TIME
  end

  def up
    write_command '1,47,17,47,17,15,49,47,17,15,49,15,49,15,49,15,49,47,17,15,49,15,49,15,4000'
  end

  def down
    write_command '1,47,17,47,17,15,49,47,17,15,49,15,49,47,17,15,49,15,49,15,49,15,49,15,4000'
  end

  def stop
    write_command '1,47,17,47,17,15,49,47,17,15,49,15,49,15,49,47,17,15,49,15,49,15,49,15,4000'
  end

  def write_command(command)
    tries = 3

    begin
      id = rand(2 ** 16)

      @socket.write "sendir,1:#{@port},#{id},38000,1,#{command}\r"

      expected_response = "completeir,1:#{@port},#{id}"
      actual_response = @socket.readline("\r").chomp

      if expected_response != actual_response
        raise "Expected response '#{expected_response}', got '#{actual_response}'."
      end
    rescue
      raise if (tries -= 1).zero?
      sleep 0.1
      retry
    end
  end
  private :write_command
end

class Device
  include Observable

  STATE_FILE = Pathname(__dir__)/'state.json'

  UP_TIME = 10
  DOWN_TIME = 10

  def initialize(real_device)
    @state = JSON.parse(STATE_FILE.read)['state'] rescue 0
    @status = nil
    @real_device = real_device

    @up_time = @real_device&.up_time || UP_TIME
    @down_time = @real_device&.down_time || DOWN_TIME
  end

  def state
    @state
  end

  def state=(value)
    if state != value
      puts "DOOR STATE: #{value}"
      @state = value
      STATE_FILE.write JSON.generate(state: value)
      changed
    end
  end

  def up
    return if @status == :going_up
    @status = :going_up

    @thread&.kill
    @thread = Thread.new do
      @real_device&.up

      loop do
        break if state == 100
        sleep @up_time / 100.0
        self.state += 1
      end

      changed
      notify_observers(state, :up)
    end
  end

  def down
    return if @status == :going_down
    @status = :going_down

    @thread&.kill
    @thread = Thread.new do
      @real_device&.down

      loop do
        break if state == 0
        sleep @down_time / 100.0
        self.state -= 1
      end

      changed
      notify_observers(state, :down)
    end
  end

  def stop
    return if @status == :stopping
    @status = :stopping

    @thread&.kill
    @thread = Thread.new do
      @real_device&.stop

      changed
      notify_observers(state, :stopped)
    end
  end
end

class Controller
  def initialize(arduino, door)
    @state = nil
    @arduino = arduino
    @door = door

    @door.add_observer(self)
  end

  def update(state, state_sym)
    puts "Device is now #{state_sym}."
    @state == state_sym
    @arduino.write "command: stop\n"
  end

  def run
    while line = @arduino.gets
      puts line

      if command = line[/^command: (.*)$/, 1]&.chomp&.to_sym
        case command
        when :up, :stop, :down
          puts "Controller received command: #{command.inspect}"
          @door.public_send(command)
        else
          warn "Unknown command: #{command.inspect}"
        end
      end
    end
  end
end

trap 'SIGINT' do
  exit
end

real_device = ProjectionScreen.new(ITACH_IP, ITACH_PORT) rescue nil

arduino = Serial.new SERIAL_PORT, BAUD_RATE
door = Device.new(real_device)

controller = Controller.new(arduino, door)
controller.run
