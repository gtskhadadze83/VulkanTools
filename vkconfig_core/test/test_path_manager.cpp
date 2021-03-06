/*
 * Copyright (c) 2020 Valve Corporation
 * Copyright (c) 2020 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author:
 * - Christophe Riccio <christophe@lunarg.com>
 */

#include "../path_manager.h"
#include "../util.h"

#include <QDir>
#include <QFileInfo>

#include <gtest/gtest.h>

static PathManager CreatePathManager(const QString& path_value) {
    PathManager paths;
    for (int i = 0, n = PATH_COUNT; i < n; ++i) {
        const Path path = static_cast<Path>(i);
        paths.SetPath(path, path_value);
    }
    return paths;
}

static QString InitPath(const char* tail) {
    const QDir dir(QString("vkconfig/test_path_manager/") + tail);
    const QString native_path = QDir::toNativeSeparators(dir.absolutePath());
    return native_path;
}

TEST(test_path_manager, init_first) {
    const QString path_value = InitPath("init_first");
    const PathManager& paths = CreatePathManager(path_value);

    EXPECT_STREQ(path_value.toUtf8().constData(), paths.GetPath(PATH_FIRST));
}

TEST(test_path_manager, init_last) {
    const QString path_value = InitPath("init_last");
    const PathManager& paths = CreatePathManager(path_value);

    EXPECT_STREQ(path_value.toUtf8().constData(), paths.GetPath(PATH_LAST));
}

TEST(test_path_manager, init_all) {
    PathManager paths;
    paths.Clear();

    for (int i = PATH_FIRST, n = PATH_LAST; i <= n; ++i) {
        const Path path = static_cast<Path>(i);

        QString init_path = InitPath("init_all_%d");
        std::string path_string = format(init_path.toUtf8().constData(), i);

        paths.SetPath(path, path_string.c_str());

        EXPECT_STREQ(path_string.c_str(), paths.GetPath(path));
    }
}

TEST(test_path_manager, path_format) {
    static const char* table[] = {
        "/vkconfig/test\\path/format/",  "/vkconfig/test\\path/format\\", "/vkconfig\\test/path/format/",
        "/vkconfig\\test/path\\format/", "/vkconfig\\test/path/format",   "/vkconfig/test/path/format",
        "\\vkconfig\\test/path\\format", "/vkconfig/test/path/format/",   "\\vkconfig\\test/path\\format\\"};

    for (std::size_t i = 0, n = countof(table); i < n; ++i) {
        PathManager paths;
        paths.Clear();
        paths.SetPath(PATH_CONFIGURATION, QDir::homePath() + table[i]);

        const QString path = paths.GetPath(PATH_CONFIGURATION);
        const QString home_path(paths.GetPath(PATH_HOME));

        const bool is_windows_path = path == (home_path + "\\vkconfig\\test\\path\\format");
        const bool is_unix_path = path == (home_path + "/vkconfig/test/path/format");

        EXPECT_NE(is_windows_path, is_unix_path);
        EXPECT_TRUE(is_windows_path || is_unix_path);
    }
}

// Test that GetPath return the home directory when the stored path is empty
TEST(test_path_manager, empty_home) {
    PathManager paths;
    paths.Clear();

    EXPECT_STRNE(paths.GetPath(PATH_HOME), "");
    EXPECT_STREQ(paths.GetPath(PATH_HOME), paths.GetPath(PATH_FIRST));
}

// Test that export path is used as an alternative to import path when import path is empty
TEST(test_path_manager, empty_import) {
    PathManager paths;
    paths.Clear();
    paths.SetPath(PATH_EXPORT_CONFIGURATION, InitPath("empty_import"));

    EXPECT_STRNE(paths.GetPath(PATH_IMPORT_CONFIGURATION), paths.GetPath(PATH_HOME));
    EXPECT_STREQ(paths.GetPath(PATH_EXPORT_CONFIGURATION), paths.GetPath(PATH_IMPORT_CONFIGURATION));
}

// Test that import path is used as an alternative to export path when export path is empty
TEST(test_path_manager, empty_export) {
    PathManager paths;
    paths.Clear();
    paths.SetPath(PATH_IMPORT_CONFIGURATION, InitPath("empty_export"));

    EXPECT_STRNE(paths.GetPath(PATH_EXPORT_CONFIGURATION), paths.GetPath(PATH_HOME));
    EXPECT_STREQ(paths.GetPath(PATH_IMPORT_CONFIGURATION), paths.GetPath(PATH_EXPORT_CONFIGURATION));
}

TEST(test_path_manager, check_missing_dir) {
    PathManager paths;
    paths.Clear();
    paths.SetPath(PATH_CONFIGURATION, InitPath("check_missing_dir"));

    EXPECT_TRUE(strstr(paths.GetPath(PATH_CONFIGURATION), "check_missing_dir") != nullptr);
}

TEST(test_path_manager, check_default_filename) {
    PathManager paths;
    paths.Clear();

    const QString string = paths.GetFullPath(PATH_OVERRIDE_SETTINGS);

    EXPECT_TRUE(string.endsWith("vk_layer_settings.txt"));
}

TEST(test_path_manager, check_default_suffix) {
    PathManager paths;
    paths.Clear();

    const QString string = paths.GetFullPath(PATH_EXPORT_CONFIGURATION, "my_configuration");

    EXPECT_TRUE(string.endsWith("my_configuration.json"));
}

TEST(test_path_manager, check_with_suffix) {
    PathManager paths;
    paths.Clear();

    const QString string = paths.GetFullPath(PATH_EXPORT_CONFIGURATION, "my_configuration.json");

    EXPECT_TRUE(string.endsWith("my_configuration.json"));
}
