/*
 * ====================================================================
 * Copyright (c) 2002 The RapidSvn Group.  All rights reserved.
 *
 * This software is licensed as described in the file LICENSE.txt,
 * which you should have received as part of this distribution.
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */
#ifndef _CLEANUP_ACTION_H_INCLUDED_
#define _CLEANUP_ACTION_H_INCLUDED_

// svncpp
#include "svncpp/path.hpp"

// app
#include "action.hpp"

// forward declarations
class Tracer;

class CleanupAction : public Action
{
public:
  CleanupAction (wxWindow * parent, svn::Path & path, Tracer * tr);
  bool Perform ();
  bool Prepare ();

private:
  svn::Path m_path;

  // hide default and copy constructor
  CleanupAction ();
  CleanupAction (const CleanupAction &);
};

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../rapidsvn-dev.el")
 * end:
 */