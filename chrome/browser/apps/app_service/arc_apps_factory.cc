// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/apps/app_service/arc_apps_factory.h"

#include "base/feature_list.h"
#include "chrome/browser/apps/app_service/app_service_proxy_factory.h"
#include "chrome/browser/apps/app_service/arc_apps.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/app_list/arc/arc_app_list_prefs_factory.h"
#include "chrome/common/chrome_features.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"

namespace apps {

// static
ArcApps* ArcAppsFactory::GetForProfile(Profile* profile) {
  return static_cast<ArcApps*>(
      ArcAppsFactory::GetInstance()->GetServiceForBrowserContext(
          profile, true /* create */));
}

// static
ArcAppsFactory* ArcAppsFactory::GetInstance() {
  return base::Singleton<ArcAppsFactory>::get();
}

// static
bool ArcAppsFactory::IsEnabled() {
  return base::FeatureList::IsEnabled(features::kAppService);
}

ArcAppsFactory::ArcAppsFactory()
    : BrowserContextKeyedServiceFactory(
          "ArcApps",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(ArcAppListPrefsFactory::GetInstance());
  DependsOn(apps::AppServiceProxyFactory::GetInstance());
}

ArcAppsFactory::~ArcAppsFactory() = default;

KeyedService* ArcAppsFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new ArcApps(Profile::FromBrowserContext(context));
}

}  // namespace apps
