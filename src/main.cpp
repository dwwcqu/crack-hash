#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "types.h"
#include "user_options.h"
#include "usage.h"
#include "memory.h"
#include "hashcat.h"
#include "terminal.h"
#include "thread.h"
#include "status.h"
#include "shared.h"
#include "event.h"
#include "hashcat_main.h"
#ifdef WITH_BRAIN
#include "brain.h"
#endif

#if defined (__MINGW64__) || defined (__MINGW32__)
int _dowildcard = -1;
#endif

int main (int argc, char **argv)
{
  // this increases the size on windows dos boxes

  setup_console ();

  const time_t proc_start = time (NULL);

  // hashcat main context

  hashcat_ctx_t *hashcat_ctx = (hashcat_ctx_t *) hcmalloc (sizeof (hashcat_ctx_t));

  if (hashcat_init (hashcat_ctx, event) == -1)
  {
    hcfree (hashcat_ctx);

    return -1;
  }
  // install and shared folder need to be set to recognize "make install" use

  const char *install_folder = NULL;
  const char *shared_folder  = NULL;

  #if defined (INSTALL_FOLDER)
  install_folder = INSTALL_FOLDER;
  #endif

  #if defined (SHARED_FOLDER)
  shared_folder = SHARED_FOLDER;
  #endif

  // initialize the user options with some defaults (you can override them later)

  if (user_options_init (hashcat_ctx) == -1)
  {
    hashcat_destroy (hashcat_ctx);

    hcfree (hashcat_ctx);

    return -1;
  }

  // parse commandline parameters and check them

  if (user_options_getopt (hashcat_ctx, argc, argv) == -1)
  {
    user_options_destroy (hashcat_ctx);

    hashcat_destroy (hashcat_ctx);

    hcfree (hashcat_ctx);

    return -1;
  }

  if (user_options_sanity (hashcat_ctx) == -1)
  {
    user_options_destroy (hashcat_ctx);

    hashcat_destroy (hashcat_ctx);

    hcfree (hashcat_ctx);

    return -1;
  }

  // some early exits

  user_options_t *user_options = hashcat_ctx->user_options;

  #ifdef WITH_BRAIN
  if (user_options->brain_server == true)
  {
    const int rc = brain_server (user_options->brain_host, user_options->brain_port, user_options->brain_password, user_options->brain_session_whitelist, user_options->brain_server_timer);

    hcfree (hashcat_ctx);

    return rc;
  }
  #endif

  if (user_options->version == true)
  {
    printf ("%s\n", VERSION_TAG);

    user_options_destroy (hashcat_ctx);

    hashcat_destroy (hashcat_ctx);

    hcfree (hashcat_ctx);

    return 0;
  }

  // init a hashcat session; this initializes backend devices, hwmon, etc

  welcome_screen (hashcat_ctx, VERSION_TAG);

  int rc_final = -1;

  if (hashcat_session_init (hashcat_ctx, install_folder, shared_folder, argc, argv, COMPTIME) == 0)
  {
    if (user_options->usage == true)
    {
      usage_big_print (hashcat_ctx);

      rc_final = 0;
    }
    else if (user_options->hash_info == true)
    {
      hash_info (hashcat_ctx);

      rc_final = 0;
    }
    else if (user_options->backend_info > 0)
    {
      // if this is just backend_info, no need to execute some real cracking session

      backend_info (hashcat_ctx);

      rc_final = 0;
    }
    else
    {
      // now execute hashcat

      backend_info_compact (hashcat_ctx);

      user_options_info (hashcat_ctx);

      rc_final = hashcat_session_execute (hashcat_ctx);
    }
  }

  // finish the hashcat session, this shuts down backend devices, hwmon, etc

  hashcat_session_destroy (hashcat_ctx);

  // finished with hashcat, clean up

  const time_t proc_stop = time (NULL);

  goodbye_screen (hashcat_ctx, proc_start, proc_stop);

  hashcat_destroy (hashcat_ctx);

  hcfree (hashcat_ctx);

  return rc_final;
}
