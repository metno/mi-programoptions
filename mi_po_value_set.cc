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

namespace miutil {
namespace program_options {

bool value_set::is_set(option_cx opt) const
{
  return opt && (values_.find(opt) != values_.end());
}

void value_set::put_implicit(option_cx opt)
{
  if (!opt)
    throw option_error("option is null");
  if (!opt->has_implicit_value())
    throw option_error("option '" + opt->key() + "' does not have an implicit value");
  if (opt->narg() != 1)
    throw option_error("option '" + opt->key() + "' expects != 1 values, cannot set to implicit value");
  put(opt, opt->implicit_value());
}

void value_set::put(option_cx opt, const string_v& values)
{
  if (!opt)
    throw option_error("option is null");
  if (opt->is_composing()) {
    if (values.size() != 1)
      throw option_error("option '" + opt->key() + "' is composing, cannot #values != 1");
  } else {
    if (is_set(opt) && !opt->is_overwriting())
      throw option_error("option '" + opt->key() + "' already set and not composing or overwriting");
  }

  string_v& v = values_[opt];
  if (opt->is_overwriting())
    v.clear();
  v.insert(v.end(), values.begin(), values.end());
}

const string_v* value_set::get(option_cx opt) const
{
  if (!opt)
    throw option_error("option is null");
  values_t::const_iterator it = values_.find(opt);
  if (it != values_.end())
    return &it->second;
  return nullptr;
}

const string_v& value_set::values(option_cx opt) const
{
  if (const string_v* values = get(opt))
    return *values;

  throw option_error("option '" + opt->key() + "' not set and without default");
}

const std::string& value_set::value(option_cx opt, size_t index) const
{
  if (const string_v* values = get(opt))
    return values->at(index);

  if (!opt)
    throw option_error("option is null");
  if (index == 0 && opt->has_default_value())
    return opt->default_value();

  throw option_error("option '" + opt->key() + "' not set and without default");
}

option_cx value_set::find(const std::string& key, bool use_shortkey) const
{
  for (const auto& o : values_) {
    const std::string& k = use_shortkey ? o.first->shortkey() : o.first->key();
    if (!k.empty() && k == key)
      return o.first;
  }
  return nullptr;
}

void value_set::add(const value_set& other)
{
  for (const auto& o : other.values_) {
    if (!values_.insert(o).second)
      throw option_error("option '" + o.first->key() + "' already set");
  }
}

} // namespace program_options
} // namespace miutil
