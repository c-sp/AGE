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

import {ChangeDetectionStrategy, Component, Input} from "@angular/core";
import {faAngleLeft} from "@fortawesome/free-solid-svg-icons/faAngleLeft";
import {AgeNavigationService} from "../routing";
import {IAgeOnlineRom} from "./age-rom-library-contents.component";


@Component({
    selector: "age-rom-library",
    template: `
        <mat-toolbar *ngIf="showToolbar"
                     [color]="'primary'">

            <age-toolbar-action [icon]="iconBackToEmulation"
                                (clicked)="backToEmulation()"></age-toolbar-action>

        </mat-toolbar>

        <age-rom-library-contents (romClicked)="runRom($event)"
                                  [justifyContent]="'center'"></age-rom-library-contents>
    `,
    styles: [`
        :host {
            display: block;
            max-width: 80em;
            margin-left: auto;
            margin-right: auto;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeRomLibraryComponent {

    readonly iconBackToEmulation = faAngleLeft;

    @Input() showToolbar = false;

    constructor(private readonly _navigationService: AgeNavigationService) {
    }

    runRom(onlineRom: IAgeOnlineRom) {
        this._navigationService.navigateToOpenRomUrl(onlineRom.romUrl);
    }

    backToEmulation(): void {
        this._navigationService.navigateToRoot();
    }
}
