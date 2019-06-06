//
// Copyright 2018 Christoph Sprenger
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

import {NgModule} from "@angular/core";
import {BrowserModule} from "@angular/platform-browser";
import {ServiceWorkerModule} from "@angular/service-worker";

import {FontAwesomeModule} from "@fortawesome/angular-fontawesome";
import {AgeLibModule} from "age-lib";
import {environment} from "../../environments/environment";

import {AgeAppComponent} from "./age-app.component";
import {AgeSplashScreenComponent} from "./age-splash-screen.component";
import {AgeInfoComponent} from "./info/age-info.component";
import {AgeRepoCommitLinkComponent} from "./info/age-repo-commit-link.component";
import {AgeRepoLinkComponent} from "./info/age-repo-link.component";
import {AgeOpenRomLocalComponent} from "./open-rom/age-open-rom-local.component";
import {AgeOpenRomUrlComponent} from "./open-rom/age-open-rom-url.component";
import {AgeOpenRomComponent} from "./open-rom/age-open-rom.component";
import {AgeTitleBarComponent} from "./title-bar/age-title-bar.component";


@NgModule({
    imports: [
        BrowserModule,
        ServiceWorkerModule.register("ngsw-worker.js", {enabled: environment.production}),

        FontAwesomeModule,

        AgeLibModule,
    ],
    declarations: [
        AgeInfoComponent,
        AgeRepoCommitLinkComponent,
        AgeRepoLinkComponent,

        AgeOpenRomComponent,
        AgeOpenRomLocalComponent,
        AgeOpenRomUrlComponent,

        AgeTitleBarComponent,

        AgeAppComponent,
        AgeSplashScreenComponent,
    ],
    bootstrap: [AgeAppComponent],
})
export class AgeAppModule {
}
