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

#ifndef MI_PROGRAMOPTIONS_H
#define MI_PROGRAMOPTIONS_H

//#define USE_REGEX_FOR_PARSE_COMMAND_LINE

#include <iosfwd>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace miutil {
namespace program_options {

struct option_error : public std::runtime_error
{
  option_error(const std::string& what)
      : std::runtime_error(what)
  {
  }
};

class option;
typedef option* option_x;
typedef const option* option_cx;

typedef std::vector<std::string> string_v;

class option
{
public:
  option(const std::string& key, const std::string& help);
  option(const option& o);
  option(option&& o);
  ~option();

  option& operator=(const option& o);

  option& add_key(const std::string& sk);
  const std::string& key() const;
  const std::vector<std::string>& keys() const;

  const std::string& help() const;

  option& set_shortkey(const std::string& sk);
  option& add_shortkey(const std::string& sk);
  const std::string& shortkey() const;
  const std::vector<std::string>& shortkeys() const;

  bool match(const std::string& key, bool use_shortkeys) const;

  option& set_default_value(const std::string& d);
  bool has_default_value() const;
  const std::string& default_value() const;

  option& set_implicit_value(const std::string& i);
  bool has_implicit_value() const;
  const std::string& implicit_value() const;

  option& set_composing();
  bool is_composing() const;

  option& set_overwriting();
  bool is_overwriting() const;

  option& set_narg(size_t n);
  size_t narg() const;

private:
  struct d;
  std::unique_ptr<d> d_;
};

class value_set
{
public:
  bool is_set(option_cx opt) const;
  bool is_set(const option& opt) const { return is_set(&opt); }
  const string_v* get(option_cx opt) const;
  const string_v* get(const option& opt) const { return get(&opt); }

  const string_v& values(option_cx opt) const;
  const string_v& values(const option& opt) const { return values(&opt); }

  const std::string& value(option_cx opt, size_t index = 0) const;
  const std::string& value(const option& opt, size_t index = 0) const { return value(&opt, index); }

  option_cx find(const std::string& key, bool use_shortkey = false) const;

  void put_implicit(option_cx opt);
  void put(option_cx opt, const string_v& values);
  void put(option_cx opt, const std::string& value) { put(opt, string_v(1, value)); }

  void add(const value_set& other);

private:
  typedef std::map<option_cx, string_v> values_t;
  values_t values_;
};

class option_set
{
public:
  option_set& add(const option& option)
  {
    options_.push_back(&option);
    return *this;
  }
  option_set& operator<<(const option& option) { return add(option); }

  option_cx find_option(const std::string& key, bool use_shortkey = false);

  void dump(std::ostream& out, const value_set& values) const;
  void help(std::ostream& out) const;

private:
  std::vector<option_cx> options_;
};

class positional_args_consumer
{
public:
  positional_args_consumer(value_set& vm, const string_v& pa) : values_(vm), positional_(pa), pbegin_(positional_.begin()) {}

  positional_args_consumer& operator>>(const option& opt);
  string_v::const_iterator begin() const { return pbegin_; }
  string_v::const_iterator end() const { return positional_.end(); }
  bool done() const { return begin() == end(); }

  void dump(std::ostream& out) const;

private:
  value_set& values_;
  const string_v& positional_;
  string_v::const_iterator pbegin_;
};

value_set parse_config_file(const std::string& filename, option_set& options);
value_set parse_config_file(std::istream& infile, option_set& options);

value_set parse_command_line(const std::vector<std::string>& argv, option_set& options, std::vector<std::string>& positional);
value_set parse_command_line(int argc, char* argv[], option_set& options, std::vector<std::string>& positional);

} // namespace program_options
} // namespace miutil

#endif // MI_PROGRAMOPTIONS_H
