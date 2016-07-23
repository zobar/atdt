package require ssh

proc IncomingConnection {bind session} {
    $session setCallback auth none [list AuthNone $session]
    $session handleKeyExchange
}

proc AuthNone {session message} {
    puts "auth none $session $message, user is [$message user]"
    after 2500 [list $message accept]
}

set bind [ssh::bind new -port 2222 -rsakey tmp/id_rsa]
$bind setCallback incomingConnection [list IncomingConnection $bind]

$bind listen
puts "Listening"

after 10000 {set done true}
vwait done

$bind destroy
