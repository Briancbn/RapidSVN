/*
 * ====================================================================
 * Copyright (c) 2002, 2003 The RapidSvn Group.  All rights reserved.
 *
 * This software is licensed as described in the file LICENSE.txt,
 * which you should have received as part of this distribution.
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */

// Subversion api
#include "svn_auth.h"
#include "svn_config.h"
#include "svn_utf.h"

// svncpp
#include "context.hpp"

namespace svn
{
  svn_error_t *
  Context::prompt (const char **result, 
                   const char *prompt,
                   svn_boolean_t hide, 
                   void *baton, 
                   apr_pool_t *pool)
  {
    apr_status_t status=0;

    if (baton == NULL)
    {
      return svn_error_create (status, NULL, 
                               "Context::prompt: baton==NULL");
    }

    Context * context = (Context *)baton;

    if (hide==TRUE)
    {
      SVN_ERR (svn_utf_cstring_to_utf8( 
                 (const char **)result, 
                 context->m_password.c_str (), NULL, pool));
    }
    else
    {
      SVN_ERR (svn_utf_cstring_to_utf8( 
                 (const char **)result,
                 context->m_username.c_str (), NULL, pool));
    }

    return SVN_NO_ERROR;
  }
    
  Context::Context ()
    : m_pool ()
  {
    setLogin ("", "");
  }

  Context::Context (const Context & src)
    : m_pool ()
  {
    setLogin (src.m_username.c_str (), src.m_password.c_str ());
  }

  Context::Context (const char * username, const char * password)
    : m_pool ()
  {
    setLogin (username, password);
  }

  void
  Context::setLogin (const char * username, const char * password)
  {
    m_username = username;
    m_password = password;

    m_pool.renew ();

    apr_array_header_t *providers
      = apr_array_make (m_pool, 1, sizeof (svn_auth_provider_object_t *));
    svn_auth_provider_object_t *client_provider
      = (svn_auth_provider_object_t *)apr_pcalloc (m_pool, sizeof (*client_provider));

    svn_client_get_simple_provider (
      &(client_provider->vtable),
      &(client_provider->provider_baton), m_pool);

    *(svn_auth_provider_object_t **)apr_array_push (providers) = client_provider;
    svn_auth_baton_t *ab;
    svn_auth_open (&ab, providers, m_pool);
    svn_auth_set_parameter (ab, SVN_AUTH_PARAM_DEFAULT_USERNAME,
                            m_username.c_str ());
    svn_auth_set_parameter (ab, SVN_AUTH_PARAM_DEFAULT_PASSWORD,
                            m_password.c_str ());

    memset (&m_ctx, 0, sizeof (m_ctx));
    svn_config_get_config (&m_ctx.config, m_pool);
    m_ctx.auth_baton = ab;
    m_ctx.log_msg_func = log_msg;
    m_ctx.log_msg_baton = this;
    m_ctx.notify_func = notify;
    m_ctx.notify_baton = this;
  }

  svn_error_t *
  Context::log_msg (const char **log_msg, 
                    const char **tmp_file,
                    apr_array_header_t *commit_items,
                    void *baton,
                    apr_pool_t *pool)
  {
    Context * context = (Context *)baton;
    //TODO maybe change encoding
    //take a look at *subversion*client/cmdline/util.c
    *log_msg = context->m_logMessage.c_str ();
    *tmp_file = NULL;
    
    return SVN_NO_ERROR;
  }

  void 
  Context::notify (void * baton,
                   const char *path,
                   svn_wc_notify_action_t action,
                   svn_node_kind_t kind,
                   const char *mime_type,
                   svn_wc_notify_state_t content_state,
                   svn_wc_notify_state_t prop_state,
                   svn_revnum_t revision)
  {
    //TODO forward this
  }

}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
