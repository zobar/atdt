package provide ssh 0.0

package require critcl::class

namespace eval ssh {
    critcl::clibraries -lssh

    critcl::ccode {
        #include <libssh/server.h>
    }

    critcl::class define bind {
        insvariable ssh_bind ptr {} {
            instance->ptr = ssh_bind_new();
        } {
            ssh_bind_free(instance->ptr);
        }
    }
}
