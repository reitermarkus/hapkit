require_relative 'config'

task :install_toolchain do
  sh 'arduino-cli', 'core', 'install', 'arduino:avr'
end

task :build => :install_toolchain do
  sh 'arduino-cli', 'compile', '--fqbn', 'arduino:avr:uno'
end

task :flash => :build do
  sh 'arduino-cli', 'upload', '-p', SERIAL_PORT, '--fqbn', 'arduino:avr:uno'
end

task :monitor => :flash do
  sh 'python', '-m', 'serial.tools.miniterm', '--raw', '--exit-char=3', '--rts=0', '--dtr=0', SERIAL_PORT, BAUD_RATE.to_s
end

task :clean do
  rm_rf 'build'
end

task :default => :monitor
