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
 * Authors:
 * - Richard S. Wright Jr. <richard@lunarg.com>
 * - Christophe Riccio <christophe@lunarg.com>
 */

#pragma once

#ifdef _WIN32
#include <windows.h>
#include <winreg.h>
#include <Cfgmgr32.h>
#define WIN_BUFFER_SIZE 1024
#endif
#include <QString>
#include <QVector>
#include <QSettings>
#include <QDir>

#include <vulkan/vulkan.h>

#include "layerfile.h"
#include "configuration.h"

#define DONT_SHOW_AGAIN_MESSAGE "Do not show again"
#define APP_SHORT_NAME "vkconfig"

////////////////////////////////////////////////////////////////////////////////
/// \brief The CPathFinder class
/// Going back and forth between the Windows registry and looking for files
/// in specific folders is just a mess. This class consolidates all that into
/// one single abstraction that knows whether to look in the registry or in
/// a folder with QDir.
/// This is a little weird because generally QSettings is for going back
/// and forth between the Registry or .ini files. Here, I'm going from
/// the registry to directory entries.
class PathFinder {
   public:
#ifdef _WIN32
    PathFinder(const QString& path, bool force_file_system = false);
#else
    PathFinder(const QString& path, bool force_file_system = true);
#endif
    int FileCount() { return file_list_.size(); }
    QString GetFileName(int i) { return file_list_[i]; }

   private:
    QStringList file_list_;
};

#define VKCONFIG_NAME "Vulkan Configurator"

// Saved settings for the application
#define VKCONFIG_KEY_LAUNCHAPP "launchApp"
#define VKCONFIG_KEY_ACTIVEPROFILE "activeProfile"
#define VKCONFIG_KEY_CUSTOM_PATHS "customPaths"
#define VKCONFIG_KEY_OVERRIDE_ACTIVE "overrideActive"
#define VKCONFIG_KEY_APPLY_ONLY_TO_LIST "applyOnlyToList"
#define VKCONFIG_KEY_KEEP_ACTIVE_ON_EXIT "keepActiveOnExit"
#define VKCONFIG_KEY_FIRST_RUN "firstRun"
#define VKCONFIG_KEY_APPLY_ALL "applytoall"
#define VKCONFIG_HIDE_RESTART_WARNING "restartWarning"

// This is a master list of layer settings. All the settings
// for what layers can have user modified settings. It contains
// the names of the layers and the properties of the settings.
// THIS IS TO BE READ ONLY, as it is copied from frequently
// to reset or initialize the a full layer definition for the
// profiles.
struct LayerSettingsDefaults {
    QString layer_name;                        // Name of layer
    QVector<LayerSettings*> default_settings;  // Default settings for this layer
};

//////////////////////////////////////////////////////////
// We will maintain a list of applicitons, each can have
// it's own working folder (when run in test mode), and
// it's own set of command line arguments
struct Application {
    QString executable_path;
    QString working_folder;
    QString arguments;
    QString log_file;
    bool override_layers;
};

struct Settings {};

class Configurator {
   public:
    static Configurator& Get();

    static const int DefaultConfigurationsCount;
    static const char* DefaultConfigurations[8];

    // Need this to check vulkan loader version
    uint32_t vulkan_instance_version;

    /////////////////////////////////////////////////////////////////////////
    // Just Vulkan Configurator settings
    void LoadSettings();
    void SaveSettings();
    void ResetToDefaultSettings();
    QString qsLastLaunchApplicationWPath;  // This is to match up with the application list
    bool bOverrideActive;                  // Do we have an active override?
    bool bApplyOnlyToList;                 // Apply the overide only to the application list
    bool bKeepActiveOnExit;                // Stay active when app closes
    bool bHasOldLoader;                    // Older loader does not support per-application overrides

    QString qsProfileFilesPath;      // Where config working files live
    QString qsOverrideSettingsPath;  // Where settings go when profile is active
    QString qsOverrideJsonPath;      // Where json goes when profile is active

    /////////////////////////////////////////////////////////////////////////
    // Additional places to look for layers
    void LoadCustomLayersPaths();
    void SaveCustomLayersPaths();
    QStringList custom_layers_paths;

    /////////////////////////////////////////////////////////////////////////
    // The list of applications affected
    QVector<Application*> overridden_application_list;
    void LoadOverriddenApplicationList();
    void SaveOverriddenApplicationList();

    ////////////////////////////////////////////////////////////////////////
    // A readonly list of layer names with the associated settings
    // and their default values. This is for reference by individual profile
    // objects.
    QVector<LayerSettingsDefaults*> default_layers_settings;
    void LoadDefaultLayerSettings();
    const LayerSettingsDefaults* FindLayerSettings(QString layer_name);
    void LoadDefaultSettings(LayerFile* empty_layer);

    ////////////////////////////////////////////////////////////////////////
    // Look for all installed layers. This contains their path, version info, etc.
    // but does not contain settings information. The default settings are stored
    // in the above (defaultLayerSettings). The binding of a layer with it's
    // particular settings is done in the profile (Configuration - in configuration list).
    // This includes all found implicit, explicit, or layers found in custom folders
    QVector<LayerFile*> available_Layers;  // All the found layers, lumped together
    void FindAllInstalledLayers();
    void LoadLayersFromPath(const QString& path, QVector<LayerFile*>& layer_list, LayerType type);
    const LayerFile* FindLayerNamed(QString layer_name, const char* location = nullptr);

    QVector<Configuration*> available_configurations;

    // We need to push and pop a temporary environment.
    // The stack is only one deep...
    Configuration* saved_configuration;

    void PushConfiguration(Configuration* configuration);
    void PopConfiguration();

    Configuration* CreateEmptyConfiguration();
    Configuration* FindConfiguration(QString configuration_name);
    Configuration* LoadConfiguration(QString path_configuration);  // Load .profile descriptor
    void LoadAllConfigurations();                                  // Load all the .profile files found
    bool SaveConfiguration(Configuration* configuration);          // Write .profile descriptor
    void ImportConfiguration(QString full_path_to_source);
    void ExportConfiguration(QString full_path_to_source, QString full_path_to_dest);

    void FindVkCube();

    bool HasLayers() const;
    bool IsRunningAsAdministrator() { return running_as_administrator_; }

    // Set this as the current override configuration
    void SetActiveConfiguration(Configuration* configuration);
    Configuration* GetActiveConfiguration() { return active_configuration_; }
    void RefreshConfiguration() {
        if (active_configuration_) SetActiveConfiguration(active_configuration_);
    }

    QString GetConfigurationPath() { return qsProfileFilesPath; }

    QString CheckVulkanSetup();
    void CheckApplicationRestart();

   private:
    Configurator();
    ~Configurator();
    Configurator(const Configurator&) = delete;
    Configurator& operator=(const Configurator&) = delete;

    Configuration* active_configuration_;

    bool running_as_administrator_;  // Are we being "Run as Administrator"
    bool first_run_;                 // This is used for populating the initial set of configurations

    void ClearLayerLists();

#ifdef _WIN32
    void LoadDeviceRegistry(DEVINST id, const QString& entry, QVector<LayerFile*>& layerList, LayerType type);
    void LoadRegistryLayers(const QString& path, QVector<LayerFile*>& layerList, LayerType type);

    void AddRegistryEntriesForLayers(QString qsJSONFile, QString qsSettingsFile);
    void RemoveRegistryEntriesForLayers(QString qsJSONFile, QString qsSettingsFile);
#endif
};
