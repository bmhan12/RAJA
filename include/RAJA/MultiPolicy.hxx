/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   RAJA wrapper for multiple policies and dynamic selection
 *
 ******************************************************************************
 */

#ifndef RAJA_MultiPolicy_HXX
#define RAJA_MultiPolicy_HXX

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-689114
//
// All rights reserved.
//
// This file is part of RAJA.
//
// For additional details, please also read RAJA/LICENSE.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the disclaimer below.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "RAJA/RAJA.hxx"

#include <tuple>

namespace RAJA
{
/// MultiPolicy - Meta-policy for choosing between a compile-time list of
/// policies at runtime
///
/// \tparam Selector Functor/Lambda/function type used to select policies
/// \tparam Policies Variadic pack of policies, numbered from 0
template <typename Selector, typename ...Policies>
class MultiPolicy {
  Selector s;
public:
  MultiPolicy() = delete; // No default construction
  MultiPolicy(Selector s) : s(s)
  { }

  MultiPolicy(const MultiPolicy &p) : s(p.s)
  { }

  template <typename Iterable>
  int operator()(Iterable &&i)
  {
    return s(i);
  }
};

/// make_multi_policy - Construct a MultiPolicy from the given selector and
/// Policies
///
/// \tparam Policies list of policies, 0 to N-1
/// \tparam Selector type of s, should almost always be inferred
/// \param s a functor called with the segment object passed to
/// forall, must return an int in the set 0 to N-1 selecting the policy to use
/// \return A MultiPolicy containing the given selector s
template <typename ...Policies, typename Selector>
auto make_multi_policy(Selector s) -> MultiPolicy<Selector, Policies...>
{
  return MultiPolicy<Selector, Policies...>(s);
}

namespace detail
{
/// @brief policy_invoker - internal helper for compile-time expansion and
/// runtime selection
template <size_t index, size_t size, typename first, typename... rest>
struct policy_invoker {
  using policy = first;

  template <typename Iterable, typename Body>
  static void invoke(int offset, Iterable &&iter, Body &&body)
  {
    if(offset == size - index - 1) {
      forall(policy(), iter, body);
    } else {
      policy_invoker<index - 1, size, rest...>::invoke(offset, iter, body);
    }
  }
};

template <size_t size, typename first, typename... rest>
struct policy_invoker<0, size, first, rest...> {
  using policy = first;

  template <typename Iterable, typename Body>
  static void invoke(int offset, Iterable &&iter, Body &&body)
  {
    if(offset == size - 1) {
      forall(policy(), iter, body);
    } else {
      throw std::runtime_error("unknown offset invoked");
    }
  }
};


}

template <typename ...Policies, typename Selector, typename Iterable,
          typename Body>
RAJA_INLINE void forall(MultiPolicy<Selector, Policies...> p, Iterable &&iter,
                        Body &&body)
{
  using inv = detail::policy_invoker<sizeof...(Policies) - 1, sizeof...
      (Policies), Policies...>;
  inv::invoke(p(iter), iter, body);
}

}

#endif
