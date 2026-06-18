
#include "mi_cpptest.h"

#include "mi_programoptions.h"

#include <sstream>

using namespace miutil::program_options;

MI_CPPTEST_TEST_CASE(progopt_config_file)
{
    const option o1("one.setting", "this is a setting");
    const option o2("one.option", "this is an option");
    const option o3("no_dot", "this is special");

    option_set options;
    options.add(o1).add(o2).add(o3);

    std::istringstream configfile("no_dot=5\n[one]\nsetting=hei\noption=hi\n");
    const value_set values = parse_config_file(configfile, options);

    MI_CPPTEST_CHECK(values.is_set(&o1));
    MI_CPPTEST_CHECK_EQ("hi", values.value(&o2));
    MI_CPPTEST_CHECK_EQ("5", values.value(&o3));
}

MI_CPPTEST_TEST_CASE(progopt_cmdline_and_dump)
{
  const option o1 = option("one.setting", "this is a setting").set_composing();
  const option o2("one.option", "this is an option");

  option_set options;
  options.add(o1).add(o2);

  const char* argv[] = { "test.exe", "--one.setting", "hei", "--one.setting=hi", "hey" };
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  const value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  MI_CPPTEST_CHECK_EQ("hei", values.value(&o1, 0));
  MI_CPPTEST_CHECK_EQ("hi", values.value(&o1, 1));
  MI_CPPTEST_CHECK(!values.is_set(&o2));
  MI_CPPTEST_CHECK_EQ(1, positional.size());
  MI_CPPTEST_CHECK_EQ("hey", positional[0]);

  std::ostringstream dump;
  options.dump(dump, values);
  MI_CPPTEST_CHECK_EQ("--one.setting\n  => 'hei'\n  => 'hi'\n", dump.str());
}

MI_CPPTEST_TEST_CASE(progopt_values_add)
{
  const option o1("one.setting", "this is a setting");
  const option o2("one.option", "this is an option");

  option_set options;
  options.add(o1).add(o2);

  std::istringstream configfile1("[one]\nsetting=hei\n");
  const value_set values1 = parse_config_file(configfile1, options);
  MI_CPPTEST_CHECK(values1.is_set(&o1));

  std::istringstream configfile2("[one]\noption=hi\n");
  const value_set values2 = parse_config_file(configfile2, options);
  MI_CPPTEST_CHECK_EQ("hi", values2.value(&o2));

  value_set values = values1;
  values.add(values2);
  MI_CPPTEST_CHECK_EQ("hei", values.value(&o1));
  MI_CPPTEST_CHECK_EQ("hi", values.value(&o2));
  MI_CPPTEST_CHECK_EQ(&o2, values.find("one.option", false));
  MI_CPPTEST_CHECK_EQ(values.find("no.way", false), nullptr);
}

MI_CPPTEST_TEST_CASE(progopt_help)
{
  const option o1 = option("one.setting", "this is a setting").set_shortkey("os");
  const option o2 = option("one.option", "this is an option").set_default_value("hi");

  option_set options;
  options.add(o1).add(o2);

  std::ostringstream help;
  options.help(help);
  MI_CPPTEST_CHECK_EQ("--one.setting / -os: this is a setting\n--one.option: this is an option (default: hi)\n", help.str());
}

MI_CPPTEST_TEST_CASE(progopt_short)
{
  const option o1 = option("one.setting", "this is a setting").set_composing().set_shortkey("os");
  const option o2("one.option", "this is an option");

  option_set options;
  options.add(o1).add(o2);

  const char* argv[] = { "test.exe", "-os", "hei", "--one.setting=hi" };
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  const value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  MI_CPPTEST_CHECK_EQ("hei", values.value(&o1, 0));
  MI_CPPTEST_CHECK_EQ("hi", values.value(&o1, 1));
  MI_CPPTEST_CHECK(!values.is_set(&o2));
}

MI_CPPTEST_TEST_CASE(progopt_narg)
{
  const option o1 = option("help", "show help").set_shortkey("h").set_narg(0);
  const option o2 = option("this.option", "this expects two").set_shortkey("to").set_narg(2);

  option_set options;
  options.add(o1).add(o2);

  const char* argv[] = { "test.exe", "-to", "hei", "ho", "-h", "extra" };
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  const value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  MI_CPPTEST_CHECK(values.is_set(&o1));
  MI_CPPTEST_CHECK_EQ("hei", values.value(&o2, 0));
  MI_CPPTEST_CHECK_EQ("ho", values.value(&o2, 1));
  MI_CPPTEST_CHECK_EQ("extra", positional.at(0));
}

MI_CPPTEST_TEST_CASE(progopt_consume_positional)
{
  const option o1 = option("one.setting", "this is a setting").set_composing();
  const option o2("one.option", "this is an option");

  option_set options;
  options.add(o1).add(o2);

  const char* argv[] = { "test.exe", "--one.setting", "hei", "--one.setting=hi", "hey" };
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  MI_CPPTEST_CHECK_EQ("hei", values.value(&o1, 0));
  MI_CPPTEST_CHECK_EQ("hi", values.value(&o1, 1));
  MI_CPPTEST_CHECK(!values.is_set(&o2));

  positional_args_consumer pac(values, positional);
  pac >> o2;
  MI_CPPTEST_CHECK(pac.done());
  MI_CPPTEST_CHECK_EQ("hey", values.value(o2));
}

MI_CPPTEST_TEST_CASE(progopt_end_of_options)
{
  const option o1 = std::move(option("one.setting", "this is a setting").set_composing());
  const option o2("one.option", "this is an option");

  option_set options;
  options.add(o1).add(o2);

  const char* argv[] = { "test.exe", "--one.setting", "hei", "--", "--one.option=hi", "hey" };
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  MI_CPPTEST_CHECK_EQ(1, values.values(&o1).size());
  MI_CPPTEST_CHECK_EQ(2, positional.size());
}

MI_CPPTEST_TEST_CASE(progopt_multiple_keys)
{
  const option o1 = option("one.setting", "this is a setting").add_key("one.choice");

  option_set options;
  options.add(o1);

  std::istringstream configfile1("[one]\nchoice=hei\n");
  const value_set values1 = parse_config_file(configfile1, options);
  MI_CPPTEST_CHECK(values1.is_set(&o1));
}

MI_CPPTEST_TEST_CASE(progopt_multiple_shortkeys)
{
  const option o2 = option("one.option", "this is an option").add_shortkey("option");

  option_set options;
  options.add(o2);

  std::vector<std::string> cmdline {"test.exe", "-option=hi"};
  string_v positional;
  const value_set values2 = parse_command_line(cmdline, options, positional);
  MI_CPPTEST_CHECK_EQ("hi", values2.value(&o2));
}

MI_CPPTEST_TEST_CASE(progopt_bad_duplicate)
{
  const option os = option("setup", "this is an option").add_shortkey("s");

  option_set options;
  options.add(os);

  std::vector<std::string> cmdline {"test.exe", "--setup=ha", "-s", "he"};
  string_v positional;
  MI_CPPTEST_CHECK_THROW(parse_command_line(cmdline, options, positional), option_error);
}

MI_CPPTEST_TEST_CASE(progopt_overwriting)
{
  const option os = std::move(option("setup", "this is an option").add_shortkey("s").set_overwriting());

  option_set options;
  options.add(os);

  std::vector<std::string> cmdline {"test.exe", "--setup=ha", "-s", "he"};
  string_v positional;
  const value_set values2 = parse_command_line(cmdline, options, positional);
  MI_CPPTEST_CHECK_EQ("he", values2.value(&os));
}

MI_CPPTEST_TEST_CASE(progopt_empty_option_value)
{
  const option o1 = option("empty_option", "option with an empty value");

  option_set options;
  options.add(o1);

  const char* argv[] = {"test.exe", "--empty_option="};
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  const value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  // Verify that the value is captured as empty
  MI_CPPTEST_CHECK_EQ("", values.value(&o1));
}

MI_CPPTEST_TEST_CASE(progopt_implicit_and_default_values_combined)
{
  const option o1 = option("setting", "option with default and implicit values").set_default_value("default").set_implicit_value("implicit");

  option_set options;
  options.add(o1);

  const char* argv[] = {"test.exe", "--setting"};
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  const value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  // Verify that the implicit value is used when no explicit value is provided
  MI_CPPTEST_CHECK_EQ("implicit", values.value(&o1));
}

MI_CPPTEST_TEST_CASE(progopt_long_option_without_value_and_default)
{
  const option o1 = option("setting", "option with default").set_default_value("default");

  option_set options;
  options.add(o1);

  const char* argv[] = {"test.exe"};
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  const value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  // Verify that the default value is used when the option is not set
  MI_CPPTEST_CHECK_EQ("default", values.value(&o1));
}

MI_CPPTEST_TEST_CASE(progopt_config_file_exception_throw)
{
  const option o1 = option("section1.setting", "this is a setting");
  const option o2 = option("section1.nested_setting", "this is a nested setting").set_implicit_value("default_nested");
  const option o3 = option("section2.option", "this is an option").set_implicit_value("default_option");
  const option o4 = option("section2.spaces_option", "this is a option with spaces");
  const option o5 = option("global.setting", "this is a global setting");

  option_set options;
  options.add(o1).add(o2).add(o3).add(o4).add(o5);

  std::istringstream configfile("# Global section\n"
                                "global.setting=global_value\n\n"
                                "[section1]\n"
                                "# A comment in section 1\n"
                                "setting=value1\n"
                                "nested_setting=\"a nested value with spaces\"\n"
                                "\n"
                                "[section2]\n"
                                "option=value2\n"
                                "spaces_option='a value with special chars!@#'\n"
                                "# Another comment in section 2\n"
                                "option=\n" // It should trigger an exception.
                                "\n"
                                "# Invalid or missing sections should be ignored");
  MI_CPPTEST_CHECK_THROW(parse_config_file(configfile, options), option_error);
}

MI_CPPTEST_TEST_CASE(progopt_option_assignment_operator)
{
  option opt1 = option("setting1", "This is the first setting");
  option opt2 = option("setting2", "This is the second setting");

  opt2.set_default_value("default_value2").set_shortkey("s2").set_implicit_value("implicit_value2").set_composing();

  opt1 = opt2;
  MI_CPPTEST_CHECK_EQ("setting2", opt1.key());
  MI_CPPTEST_CHECK_EQ("This is the second setting", opt1.help());
  MI_CPPTEST_CHECK_EQ("default_value2", opt1.default_value());
  MI_CPPTEST_CHECK_EQ("implicit_value2", opt1.implicit_value());
  MI_CPPTEST_CHECK(opt1.is_composing());
  MI_CPPTEST_CHECK_EQ("s2", opt1.shortkey());
}

MI_CPPTEST_TEST_CASE(progopt_option_set_non_existing_settings)
{
  const option opt1 = option("setting1", "This is setting1").add_shortkey("s1");
  const option opt2 = option("setting2", "This is setting2");

  option_set options;
  options.add(opt1).add(opt2);

  MI_CPPTEST_CHECK_THROW(options.find_option("non_existing_setting", false), option_error);
  MI_CPPTEST_CHECK_THROW(options.find_option("non_existing_shortkey", true), option_error);
}

// #ifndef USE_REGEX_FOR_PARSE_COMMAND_LINE
//  These tests will segfault when using the regex version
MI_CPPTEST_TEST_CASE(progopt_long_options)
{
  const option o1 = option("one.setting", "this is a very long setting");
  const option o2("one.option", "this is a very long option");

  option_set options;
  options.add(o1).add(o2);

  // Create an extremely long string
  std::string long_value(27000, 'x');
  std::string long_option = "--one.setting=" + long_value;

  const char* argv[] = {"test.exe", long_option.c_str()};
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  const value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  MI_CPPTEST_CHECK(values.is_set(&o1));
  MI_CPPTEST_CHECK_EQ(long_value, values.value(&o1));

  MI_CPPTEST_CHECK_EQ(0, positional.size());
}
// These will segfault when using the regex version
MI_CPPTEST_TEST_CASE(progopt_long_options_part2)
{
  const option o1 = option("one.setting", "this is a very long and complex setting");
  const option o2("one.option", "this is a very long and complex option");

  option_set options;
  options.add(o1).add(o2);

  std::string long_input = "value1, value2; value3, \"quoted value with spaces\"; 'single quoted', value with spaces and ; ; more,values";

  std::string long_value;
  for (int i = 0; i < 5000000; ++i) {
    long_value += long_input + ",";
  }
  std::string long_option = "--one.setting=" + long_value;

  const char* argv[] = {"test.exe", long_option.c_str()};
  const int argc = sizeof(argv) / sizeof(argv[0]);

  string_v positional;
  const value_set values = parse_command_line(argc, const_cast<char**>(argv), options, positional);

  // Verify that the long option was correctly parsed and stored
  MI_CPPTEST_CHECK(values.is_set(&o1));
  MI_CPPTEST_CHECK_EQ(long_value, values.value(&o1));

  // Ensure there are no positional arguments
  MI_CPPTEST_CHECK_EQ(0, positional.size());
}

// #endif
