/*
  mi-programoptions

  Copyright (C) 2019-2021 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no

  This file is part of mi-programoptions.

  mi-programoptions is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  mi-programoptions is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with mi-programoptions; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mi_programoptions.h"

namespace {
const std::string EMPTY;
}

namespace miutil {
namespace program_options {

struct option::d
{
  std::vector<std::string> keys_;
  std::vector<std::string> shortkeys_;
  std::string help_;
  size_t narg_;
  bool is_composing_;
  bool is_overwriting_;
  std::string default_;
  bool has_default_;
  std::string implicit_;
  bool has_implicit_;
};

option::option(const std::string& key, const std::string& help)
    : d_(new d)
{
  d_->help_ = help;
  d_->narg_ = 1;
  d_->is_composing_ = false;
  d_->is_overwriting_ = false;
  d_->has_default_ = false;
  d_->has_implicit_ = false;

  add_key(key);
}

option::option(const option& o)
    : d_(new d(*o.d_))
{
}

option::option(option&& o)
  : d_(std::move(o.d_))
{
}

option& option::operator=(const option& o)
{
  if (this != &o)
    d_.reset(new d(*o.d_));
  return *this;
}

option::~option() {}

option& option::add_key(const std::string& k)
{
  if (!k.empty())
    d_->keys_.push_back(k);
  return *this;
}

const std::string& option::key() const
{
  return d_->keys_.empty() ? EMPTY : d_->keys_.front();
}

const std::vector<std::string>& option::keys() const
{
  return d_->keys_;
}

const std::string& option::help() const
{
  return d_->help_;
}

option& option::set_shortkey(const std::string& sk)
{
  d_->shortkeys_.clear();
  return add_shortkey(sk);
}

option& option::add_shortkey(const std::string& sk)
{
  if (!sk.empty())
    d_->shortkeys_.push_back(sk);
  return *this;
}

const std::string& option::shortkey() const
{
  return d_->shortkeys_.empty() ? EMPTY : d_->shortkeys_.front();
}

const std::vector<std::string>& option::shortkeys() const
{
  return d_->shortkeys_;
}

bool option::match(const std::string& key, bool use_shortkeys) const
{
  const std::vector<std::string>& k = use_shortkeys ? d_->shortkeys_ : d_->keys_;
  return std::find(k.begin(), k.end(), key) != k.end();
}

option& option::set_default_value(const std::string& d)
{
  d_->has_default_ = true;
  d_->default_ = d;
  return *this;
}

bool option::has_default_value() const
{
  return d_->has_default_;
}

const std::string& option::default_value() const
{
  return d_->default_;
}

option& option::set_implicit_value(const std::string& i)
{
  d_->has_implicit_ = true;
  d_->implicit_ = i;
  return *this;
}

bool option::has_implicit_value() const
{
  return d_->has_implicit_;
}

const std::string& option::implicit_value() const
{
  return d_->implicit_;
}

option& option::set_composing()

{
  d_->is_composing_ = true;
  d_->is_overwriting_ = false;
  return *this;
}

bool option::is_composing() const
{
  return d_->is_composing_;
}

option& option::set_overwriting()
{
  d_->is_composing_ = false;
  d_->is_overwriting_ = true;
  return *this;
}

bool option::is_overwriting() const
{
  return d_->is_overwriting_;
}

option& option::set_narg(size_t n)
{
  d_->narg_ = n;
  return *this;
}

size_t option::narg() const
{
  return d_->narg_;
}

} // namespace program_options
} // namespace miutil
