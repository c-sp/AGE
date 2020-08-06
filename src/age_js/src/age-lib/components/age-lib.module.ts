//
// Copyright 2020 Christoph Sprenger
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

import {CommonModule} from '@angular/common';
import {HttpClientModule} from '@angular/common/http';
import {NgModule} from '@angular/core';
import {FormsModule} from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatIconModule} from '@angular/material/icon';
import {MatSliderModule} from '@angular/material/slider';
import {MatToolbarModule} from '@angular/material/toolbar';
import {AgeEmulationComponent, AgeEmulatorContainerComponent, AgeTaskStatusComponent} from './emulator';
import {
    AgeToolbarActionPlayComponent,
    AgeToolbarActionVolumeComponent,
    AgeToolbarBackgroundComponent,
    AgeToolbarSpacerComponent,
} from './toolbar';


@NgModule({
    imports: [
        CommonModule,
        FormsModule,
        HttpClientModule,

        MatButtonModule,
        MatIconModule,
        MatSliderModule,
        MatToolbarModule,
    ],
    declarations: [
        AgeEmulationComponent,
        AgeEmulatorContainerComponent,
        AgeTaskStatusComponent,

        AgeToolbarActionPlayComponent,
        AgeToolbarActionVolumeComponent,
        AgeToolbarBackgroundComponent,
        AgeToolbarSpacerComponent,
    ],
    exports: [
        AgeToolbarSpacerComponent,
        AgeEmulatorContainerComponent,
    ],
})
export class AgeLibModule {
}
