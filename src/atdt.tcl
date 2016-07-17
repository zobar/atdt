package require ssh

proc IncomingConnection {bind session} {
    puts "IncomingConnection $bind $session"

    $session setCallback auth none [list AuthNone $session]
    $session setCallback status closedError [list ClosedError $session]

    puts "queueing kex"
    after idle [list $session handleKeyExchange]
}

proc AuthNone {session message} {
    puts "auth none $session $message, user is [$message user]"
    $message accept
}

proc ClosedError {session} {
    puts "status closedError $session"
    $session destroy
}

set bind [ssh::bind new -port 2222 -rsakey tmp/id_rsa]
$bind setCallback incomingConnection [list IncomingConnection $bind]
$bind listen

puts "Listening"
after 10000 {set done true}
puts "Done"
vwait done

$bind destroy
