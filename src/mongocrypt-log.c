/*
 * Copyright 2019-present MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mongocrypt-log-private.h"

#include <bson/bson.h>

static struct _log_globals_t {
   mongocrypt_mutex_t mutex; /* protects fn and ctx. */
   mongocrypt_log_fn_t fn;
   void *ctx;
   bool trace_enabled;
} log_globals;


void
_mongocrypt_default_log_fn (mongocrypt_log_level_t level,
                            const char *message,
                            void *ctx)
{
   switch (level) {
   case MONGOCRYPT_LOG_LEVEL_FATAL:
      printf ("FATAL");
      break;
   case MONGOCRYPT_LOG_LEVEL_ERROR:
      printf ("ERROR");
      break;
   case MONGOCRYPT_LOG_LEVEL_WARNING:
      printf ("WARNING");
      break;
   case MONGOCRYPT_LOG_LEVEL_INFO:
      printf ("INFO");
      break;
   case MONGOCRYPT_LOG_LEVEL_TRACE:
      printf ("TRACE");
      break;
   }
   printf (" %s\n", message);
}

void
_mongocrypt_log_init (void)
{
   /* initialize globals. */
   mongocrypt_mutex_init (&log_globals.mutex);
   log_globals.fn = _mongocrypt_default_log_fn;
   log_globals.ctx = NULL;
   log_globals.trace_enabled = getenv ("MONGOCRYPT_TRACE");
}

void
_mongocrypt_log_set_fn (mongocrypt_log_fn_t fn, void *ctx)
{
   mongocrypt_mutex_lock (&log_globals.mutex);
   log_globals.fn = fn;
   log_globals.ctx = ctx;
   mongocrypt_mutex_unlock (&log_globals.mutex);
}

void
_mongocrypt_log (mongocrypt_log_level_t level, const char *format, ...)
{
   va_list args;
   char *message;

   if (level == MONGOCRYPT_LOG_LEVEL_TRACE && !log_globals.trace_enabled) {
      return;
   }

   BSON_ASSERT (format);

   va_start (args, format);
   message = bson_strdupv_printf (format, args);
   va_end (args);

   mongocrypt_mutex_lock (&log_globals.mutex);
   log_globals.fn (level, message, log_globals.ctx);
   mongocrypt_mutex_unlock (&log_globals.mutex);
   bson_free (message);
}