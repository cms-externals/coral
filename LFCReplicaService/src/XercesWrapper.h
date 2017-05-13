/*
 * Project: POOL
 * Sub-project: CORAL
 * Web site: http://pool.cern.ch/coral
 *
 * Author: Zsolt.Molnar@cern.ch (http://www.zsoltmolnar.hu)
 */
#ifndef LFCREPLICASERVICE_BOOSTTIMETRAITS_H_
#define LFCREPLICASERVICE_BOOSTTIMETRAITS_H_

// Disable warnings triggered by the CppUnit headers
// See http://wiki.services.openoffice.org/wiki/Writing_warning-free_code
// See also http://www.artima.com/cppsource/codestandards.html
#if defined __GNUC__
#  pragma GCC system_header
#elif defined __SUNPRO_CC
#  pragma disable_warn
#elif defined _MSC_VER
#  pragma warning(push, 1)
#endif

#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/dom/DOM.hpp"

#if defined __SUNPRO_CC
#  pragma enable_warn
#elif defined _MSC_VER
#  pragma warning(pop)
#endif

#endif /*LFCREPLICASERVICE_BOOSTTIMETRAITS_H_*/
