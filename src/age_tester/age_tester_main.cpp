//
// Copyright 2021 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "age_tester_arguments.hpp"
#include "age_tester_run_tests.hpp"

#include <algorithm>
#include <iostream>



namespace
{
    void check_run_all(age::tester::options& options)
    {
        bool run_all = !options.m_acid2 && !options.m_age && !options.m_blargg && !options.m_gambatte && !options.m_mealybug && !options.m_mooneye_gb && !options.m_same_suite;

        options.m_acid2 |= run_all;
        options.m_age |= run_all;
        options.m_blargg |= run_all;
        options.m_gambatte |= run_all;
        options.m_mealybug |= run_all;
        options.m_mooneye_gb |= run_all;
        options.m_same_suite |= run_all;
    }

    void sort_and_print(std::vector<age::tester::test_result>& tests)
    {
        std::vector<std::string> test_names;

        std::transform(begin(tests),
                       end(tests),
                       std::back_inserter(test_names),
                       [](auto& res) {
                           return res.m_test_name;
                       });

        std::sort(begin(test_names), end(test_names));

        std::for_each(begin(test_names),
                      end(test_names),
                      [](auto& name) {
                          std::cout << "    " << name << std::endl;
                      });
    }

} // namespace



int main(int argc, char** argv)
{
    age::tester::options opts = age::tester::parse_arguments(argc, argv);

    // just print the help text
    if (opts.m_help)
    {
        age::tester::print_help(argc, argv);
        return 0;
    }

    // terminate with error on unknown/invalid argument
    if (!opts.m_unknown_options.empty() || !opts.m_invalid_arg_options.empty())
    {
        std::for_each(begin(opts.m_unknown_options),
                      end(opts.m_unknown_options),
                      [&](auto& opt) {
                          std::cout << "unknown option: " << opt << std::endl;
                      });

        std::for_each(begin(opts.m_invalid_arg_options),
                      end(opts.m_invalid_arg_options),
                      [&](auto& opt) {
                          std::cout << "missing or invalid argument for option: " << opt << std::endl;
                      });

        std::cout << std::endl;
        age::tester::print_help(argc, argv);
        return 1;
    }

    // notify the user about where we're about to look for test rom files
    check_run_all(opts);
    std::cout << "test categories:"
              << (opts.m_acid2 ? " acid2" : "")
              << (opts.m_age ? " age" : "")
              << (opts.m_blargg ? " blargg" : "")
              << (opts.m_gambatte ? " gambatte" : "")
              << (opts.m_mealybug ? " mealybug-tearoom-tests" : "")
              << (opts.m_mooneye_gb ? " mooneye-gb" : "")
              << std::endl
              << "looking for test roms in: " << opts.m_test_suite_path.string()
              << std::endl;

#ifndef AGE_COMPILE_LOGGER
    opts.m_log_categories = {};
#endif
    if (opts.m_log_categories.empty())
    {
        std::cout << "test rom logs disabled" << std::endl;
    }
    else
    {
    }

    if (!opts.m_whitelist.empty())
    {
        std::cout << "whitelist: " << opts.m_whitelist << std::endl;
    }
    if (!opts.m_blacklist.empty())
    {
        std::cout << "blacklist: " << opts.m_blacklist << std::endl;
    }

    // find test rom files & schedule test tasks
    auto results = age::tester::run_tests(opts);
    if (results.empty())
    {
        std::cout << "no tests found!" << std::endl;
        return 1;
    }
    std::cout << "finished " << results.size() << " test(s)" << std::endl;

    std::vector<age::tester::test_result> passed_tests;
    std::copy_if(begin(results),
                 end(results),
                 std::back_inserter(passed_tests),
                 [](auto& res) {
                     return res.m_test_passed;
                 });

    std::cout << passed_tests.size() << " test(s) passed" << std::endl;
    if (opts.m_print_passed)
    {
        sort_and_print(passed_tests);
    }

    std::vector<age::tester::test_result> failed_tests;
    std::copy_if(begin(results),
                 end(results),
                 std::back_inserter(failed_tests),
                 [](auto& res) {
                     return !res.m_test_passed;
                 });

    std::cout << failed_tests.size() << " test(s) failed" << std::endl;
    if (opts.m_print_failed)
    {
        sort_and_print(failed_tests);
    }

    return failed_tests.empty() ? 0 : 1;
}
