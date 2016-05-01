package require ssh

ssh::bind create sshd -blocking false -myport 2222 -rsakey tmp/id_rsa
puts "listening"
