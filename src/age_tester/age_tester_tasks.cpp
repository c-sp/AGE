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

#include <age_debug.hpp>

#include "age_tester_tasks.hpp"
#include "age_tester_thread_pool.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <regex>



namespace
{
    std::vector<std::string> read_list_file(const std::filesystem::path &file_path)
    {
        std::vector<std::string> result;

        std::ifstream ifs(file_path);
        std::string line;
        while (std::getline(ifs, line))
        {
            // strip comment
            line = line.substr(0, line.find('#'));
            // trim left & right
            auto is_space = [](auto c) { return !std::isspace(c); };
            line.erase(begin(line), std::find_if(begin(line), end(line), is_space));
            line.erase(std::find_if(rbegin(line), rend(line), is_space).base(), end(line));
            // discard empty lines
            if (!line.empty())
            {
                result.emplace_back(line);
            }
        }

        return result;
    }



    typedef std::function<bool(const std::string &)> path_matcher;

    bool match_everything(const std::string &) { return true; }
    bool match_nothing(const std::string &) { return false; }

    path_matcher new_regex_matcher(const std::string &regex_string)
    {
        std::regex regex(regex_string, std::regex_constants::ECMAScript);

        return [=](const std::string &absolute_path) {
            return std::regex_search(absolute_path, regex);
        };
    }

    path_matcher new_list_matcher(const std::filesystem::path &file)
    {
        auto regex_strings = read_list_file(file);

        std::vector<path_matcher> matcher_list;
        std::transform(begin(regex_strings), end(regex_strings), std::back_inserter(matcher_list), [](auto s) { return new_regex_matcher(s); });

        return [=](const std::string &absolute_path) {
            return std::any_of(begin(matcher_list), end(matcher_list), [&](auto &m) { return m(absolute_path); });
        };
    }

    path_matcher new_matcher(const std::string &file_or_regex)
    {
        auto possible_file = std::filesystem::current_path() / file_or_regex;
        return std::filesystem::is_regular_file(possible_file)
                   ? new_list_matcher(possible_file)
                   : new_regex_matcher(file_or_regex);
    }



    void traverse_directory(const std::filesystem::path &path,
                            const std::function<void(const std::filesystem::path &)> &file_callback)
    {
        if (!std::filesystem::is_directory(path))
        {
            return;
        }
        for (auto &entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                traverse_directory(entry.path(), file_callback);
            }
            else if (entry.is_regular_file())
            {
                file_callback(entry.path());
            }
        }
    }

    void find_roms(const std::filesystem::path &path,
                   const path_matcher &use_file,
                   const std::function<void(const std::filesystem::path &)> &file_callback)
    {
        const std::array<std::string, 3> rom_file_extensions = {{".gb", ".gbc", ".cgb"}};
        auto ext_end = end(rom_file_extensions);

        traverse_directory(path, [&](const std::filesystem::path &file_path) {
            auto file_extension = file_path.extension().string();

            if (std::find(begin(rom_file_extensions), ext_end, file_extension) == ext_end)
            {
                return;
            }
            std::string path = file_path.string();
            if (std::filesystem::path::preferred_separator != '/')
            {
                std::replace(begin(path), end(path), static_cast<char>(std::filesystem::path::preferred_separator), '/');
            }
            if (use_file(path))
            {
                file_callback(file_path);
            }
        });
    }



    std::string with_hardware_indicator(const std::filesystem::path &rom_path,
                                        age::gb_hardware hardware)
    {
        auto path = rom_path.string();
        switch (hardware)
        {
            case age::gb_hardware::auto_detect:
                return path + " (auto-detect)";

            case age::gb_hardware::dmg:
                return path + " (dmg)";

            case age::gb_hardware::cgb:
                return path + " (cgb)";
        }
        return path;
    }

} // namespace



std::vector<age::tester::test_result> age::tester::run_tests(const options &opts)
{
    auto whitelist = opts.m_whitelist.length()
                         ? new_matcher(opts.m_whitelist)
                         : match_everything;

    auto blacklist = opts.m_blacklist.length()
                         ? new_matcher(opts.m_blacklist)
                         : match_nothing;

    auto matcher = [&](const std::string &path) {
        return whitelist(path) && !blacklist(path);
    };

    blocking_vector<test_result> results;
    int rom_count = 0;
    {
        thread_pool pool;

        using schedule_rom_t = std::function<void(const std::filesystem::path &, const schedule_test_t &)>;

        auto schedule_rom = [&pool, &results, &rom_count](const std::filesystem::path &rom_path,
                                                          const schedule_rom_t &schedule_rom) {
            ++rom_count;
            pool.queue_task([&pool, &results, rom_path, schedule_rom]() {
                int scheduled_count = 0;

                schedule_rom(
                    rom_path,
                    [&pool, &results, rom_path, &scheduled_count](const std::shared_ptr<age::uint8_vector> &rom_contents,
                                                                  age::gb_hardware hardware,
                                                                  const run_test_t &run) {
                        pool.queue_task([&results, rom_path, rom_contents, hardware, run]() {
                            std::unique_ptr<age::gb_emulator> emulator(new age::gb_emulator(*rom_contents, hardware));
                            auto passed = run(*emulator);
                            results.push({with_hardware_indicator(rom_path, hardware), passed});
                        });
                        ++scheduled_count;
                    });

                if (!scheduled_count)
                {
                    results.push({rom_path.string() + " (no test scheduled)", false});
                }
            });
        };

        if (opts.m_acid2)
        {
            find_roms(opts.m_test_suite_path / "cgb-acid2", matcher, [&](const std::filesystem::path &rom_path) {
                //! \todo schedule cgb-acid2
            });
            find_roms(opts.m_test_suite_path / "dmg-acid2", matcher, [&](const std::filesystem::path &rom_path) {
                //! \todo schedule dmg-acid2
            });
        }
        if (opts.m_blargg)
        {
            find_roms(opts.m_test_suite_path / "blargg", matcher, [&](const std::filesystem::path &rom_path) {
                //! \todo schedule blargg
            });
        }
        if (opts.m_gambatte)
        {
            find_roms(opts.m_test_suite_path / "gambatte", matcher, [&](const std::filesystem::path &rom_path) {
                schedule_rom(rom_path, schedule_rom_gambatte);
            });
        }
        if (opts.m_mealybug)
        {
            find_roms(opts.m_test_suite_path / "mealybug-tearoom-tests", matcher, [&](const std::filesystem::path &rom_path) {
                //! \todo schedule mealybug-tearoom-tests
            });
        }
        if (opts.m_mooneye_gb)
        {
            find_roms(opts.m_test_suite_path / "mooneye-gb", matcher, [&](const std::filesystem::path &rom_path) {
                schedule_rom(rom_path, schedule_rom_mooneye_gb);
            });
        }

        std::cout << "found " << rom_count << " rom(s)" << std::endl;
        // by letting the thread pool go out of scope we wait for it to finish
    }

    return results.copy();
}



std::shared_ptr<age::uint8_vector> age::tester::load_rom_file(const std::filesystem::path &rom_path)
{
    std::ifstream rom_file(rom_path, std::ios::in | std::ios::binary);
    return std::make_shared<age::uint8_vector>((std::istreambuf_iterator<char>(rom_file)), std::istreambuf_iterator<char>());
}