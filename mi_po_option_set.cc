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

#include <ostream>

namespace {
void show_key(std::ostream& out, miutil::program_options::option_cx opt)
{
  bool first = true;
  const std::string sep = " / ";
  for (const auto& k : opt->keys()) {
    if (!first)
      out << sep;
    out << "--" << k;
    first = false;
  }
  for (const auto& sk : opt->shortkeys()) {
    if (!first)
      out << sep;
    out << "-" << sk;
    first = false;
  }
}
} // namespace

namespace miutil {
namespace program_options {

option_cx option_set::find_option(const std::string& key, bool use_shortkey)
{
  for (option_cx opt : options_) {
    if (opt->match(key, use_shortkey))
      return opt;
  }
  throw option_error("no such option '" + key + "'");
}

void option_set::dump(std::ostream& out, const value_set& values) const
{
  for (option_cx opt : options_) {
    if (values.is_set(opt)) {
      show_key(out, opt);
      out << std::endl;
      for (const std::string& v : *values.get(opt))
        out << "  => '" << v << "'" << std::endl;
    }
  }
}

void option_set::help(std::ostream& out) const
{
  for (option_cx opt : options_) {
    show_key(out, opt);
    out << ": " << opt->help();
    if (opt->has_default_value())
      out << " (default: " << opt->default_value() << ")";
    out << std::endl;
  }
}

} // namespace program_options
} // namespace miutil
