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

#include <fstream>

namespace {
const std::string EMPTY;
}

namespace miutil {
namespace program_options {

value_set parse_config_file(const std::string& filename, option_set& options)
{
  std::ifstream infile(filename);
  if (!infile)
    throw option_error("cannot read config file '" + filename + "'");

  try {
    return parse_config_file(infile, options);
  } catch (option_error& oe) {
    throw option_error("while reading '" + filename + ": " + oe.what());
  }
}

#define OPTION_CHARS "[a-zA-Z0-9._][a-zA-Z0-9._-]*"

value_set parse_config_file(std::istream& infile, option_set& options)
{
  value_set values;
  static const std::regex re_comment("\\s*#.*");
  static const std::regex re_section("\\[(" OPTION_CHARS ")\\]$");
  enum { GS_ALL, GS_KEY };
  static const std::regex re_value("\\s*(" OPTION_CHARS ")\\s*=\\s*(.*)?\\s*");
  enum { GV_ALL, GV_KEY, GV_VALUE };
  std::string line;
  std::string section;
  for (int lineno = 1; std::getline(infile, line); ++lineno) {
    if (line.empty())
      continue;
    std::smatch match;
    if (std::regex_match(line, match, re_comment)) {
      // ignore
    } else if (std::regex_match(line, match, re_section)) {
      section = match[GS_KEY].str() + ".";
    } else if (std::regex_match(line, match, re_value)) {
      std::string value = match[GV_VALUE].str();
      if (value.size() >= 2 && (value[0] == '\'' || value[0] == '"') && value[0] == value[value.size() - 1])
        value = value.substr(1, value.size() - 2);
      try {
        values.put(options.find_option(section + match[GV_KEY].str(), false), value);
      } catch (option_error& oe) {
        std::ostringstream msg;
        msg << "lineno " << line << ": " << oe.what();
        throw option_error(msg.str());
      }
    } else {
      std::ostringstream msg;
      msg << "bad line " << lineno << ": " << line;
      throw option_error(msg.str());
    }
  }
  if (!infile.eof() && infile.bad())
    throw option_error("error reading config");
  return values;
}

value_set parse_command_line(const std::vector<std::string>& argv, option_set& options, std::vector<std::string>& positional)
{
  value_set values;
  static const std::regex re_option("(-{1,2})(" OPTION_CHARS ")(=(.*))?");
  enum { GO_ALL, GO_DASH, GO_KEY, GO_EQ_VALUE, GO_VALUE };
  bool end_of_options_marker = false;
  const int argc = argv.size();
  for (int a = 0; a < argc; ++a) {
    const std::string arg(argv[a]);
    if (arg == "--") {
      end_of_options_marker = true;
      continue;
    }
    std::smatch match;
    if (!end_of_options_marker && std::regex_match(arg, match, re_option)) {
      option_cx opt = options.find_option(match[GO_KEY], match[GO_DASH].length() == 1);
      if (match[GO_VALUE].matched) {
        if (opt->narg() == 0) {
          throw option_error("arg for no-arg option '" + opt->key() + "'");
        } else if (!opt->is_composing() && opt->narg() != 1) {
          std::ostringstream msg;
          msg << "args for option '" + opt->key() + "', which expects exactly " << opt->narg() << " values, cannot be passed with '='";
          throw option_error(msg.str());
        }
        values.put(opt, match[GO_VALUE].str());
      } else if (opt->has_implicit_value()) {
        values.put_implicit(opt);
      } else if (opt->narg() == 0) {
        values.put(opt, std::string());
      } else if (opt->is_composing() && a + 1 < argc) {
        values.put(opt, argv[++a]);
      } else if (!opt->is_composing() && a + (int)opt->narg() < argc) {
        string_v args;
        args.reserve(opt->narg());
        const int alast = a + opt->narg();
        while (a < alast)
          args.push_back(argv[++a]);
        values.put(opt, args);
      } else {
        throw option_error("no arg for option '" + opt->key() + "'");
      }
    } else {
      positional.push_back(arg);
    }
  }
  return values;
}

value_set parse_command_line(int argc, char* argv[], option_set& options, std::vector<std::string>& positional)
{
  return parse_command_line(std::vector<std::string>(argv + 1, argv + argc), options, positional);
}

positional_args_consumer& positional_args_consumer::operator>>(const option& opt)
{
  if (opt.is_composing()) {
    values_.put(&opt, *pbegin_++);
  } else if (!values_.is_set(opt)) {
    if (std::distance(begin(), end()) >= (int)opt.narg()) {
      const string_v::const_iterator opt_end = begin() + opt.narg();
      values_.put(&opt, string_v(begin(), opt_end));
      pbegin_ = opt_end;
    } else {
      throw option_error("positional arg error for option '" + opt.key() + "'");
    }
  }
  return *this;
}

void positional_args_consumer::dump(std::ostream& out) const
{
  out << "remaining positional arguments: " << std::endl;
  for (string_v::const_iterator p = pbegin_; p != positional_.end(); ++p)
    out << " -> '" << *p << "'" << std::endl;
}

} // namespace program_options
} // namespace miutil
