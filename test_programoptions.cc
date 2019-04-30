
#include "mi_cpptest.h"

#include "mi_programoptions.h"

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
  MI_CPPTEST_CHECK_EQ(nullptr, values.find("no.way", false));
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
  const option o1 = option("one.setting", "this is a setting").set_composing();
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
