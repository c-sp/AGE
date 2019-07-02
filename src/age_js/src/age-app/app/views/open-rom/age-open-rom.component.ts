//
// Copyright 2019 Christoph Sprenger
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

import {ChangeDetectionStrategy, Component, Input} from '@angular/core';
import {AgeBreakpointObserverService, AgeIconsService, IAgeLocalRomFile} from 'age-lib';
import {BehaviorSubject, combineLatest, Observable} from 'rxjs';
import {map} from 'rxjs/operators';
import {AgeNavigationService, AgeRomFileService, IAgeViewMode, MOBILE_WIDTH_PX, viewMode$} from '../../common';
import {IAgeRomCount} from './age-rom-library.component';


@Component({
    selector: 'age-open-rom',
    template: `
        <mat-toolbar *ngIf="(viewMode$ | async) as viewMode"
                     [color]="'primary'">

            <div class="hidden">
                <mat-form-field style="width: 0">
                    <input matInput>
                </mat-form-field>
            </div>

            <a *ngIf="viewMode.showLeftAngle"
               mat-button
               [routerLink]="navigationService.rootUrl">
                <mat-icon [svgIcon]="icons.faAngleLeft"></mat-icon>
            </a>

            <age-toolbar-spacer></age-toolbar-spacer>


            <div *ngIf="viewMode.toolbar === ToolbarMode.DEFAULT"
                 [ngClass]="{'toolbar-contents': !viewMode.mobileView}">

                <button mat-button (click)="setToolbarMode(ToolbarMode.SEARCH_LIBRARY)">
                    <mat-icon [svgIcon]="icons.faSearch"></mat-icon>
                </button>

                <age-toolbar-action-local-rom (openLocalRom)="openLocalRom($event, viewMode)">
                </age-toolbar-action-local-rom>

                <button mat-button (click)="setToolbarMode(ToolbarMode.ENTER_ROM_FILE_URL)">
                    <mat-icon [svgIcon]="icons.faGlobeAmericas"></mat-icon>
                </button>

                <age-toolbar-spacer *ngIf="!viewMode.mobileView"></age-toolbar-spacer>

                <a *ngIf="!viewMode.mobileView"
                   mat-button
                   [routerLink]="navigationService.rootUrl">
                    <mat-icon [svgIcon]="icons.faTimes"></mat-icon>
                </a>

            </div>


            <div *ngIf="viewMode.toolbar === ToolbarMode.SEARCH_LIBRARY"
                 cdkTrapFocus
                 [cdkTrapFocusAutoCapture]="true"
                 class="toolbar-contents">

                <form>
                    <mat-form-field [hintLabel]="romCountHint">
                        <mat-label>search library</mat-label>
                        <input matInput
                               cdkFocusInitial
                               name="libraryFilter"
                               [(ngModel)]="libraryFilter">
                    </mat-form-field>
                </form>

                <button mat-button (click)="setToolbarMode(ToolbarMode.DEFAULT)">
                    <mat-icon [svgIcon]="icons.faTimes"></mat-icon>
                </button>

            </div>


            <div *ngIf="viewMode.toolbar === ToolbarMode.ENTER_ROM_FILE_URL"
                 cdkTrapFocus
                 [cdkTrapFocusAutoCapture]="true"
                 class="toolbar-contents">

                <form>
                    <mat-form-field>
                        <mat-label>rom file url</mat-label>
                        <input matInput
                               cdkFocusInitial
                               (keydown.enter)="openRomFileUrl()"
                               name="romFileUrl"
                               [(ngModel)]="romFileUrl">
                    </mat-form-field>
                </form>

                <button mat-button (click)="setToolbarMode(ToolbarMode.DEFAULT)">
                    <mat-icon [svgIcon]="icons.faTimes"></mat-icon>
                </button>

            </div>


            <age-toolbar-spacer *ngIf="!viewMode.mobileView"></age-toolbar-spacer>

        </mat-toolbar>

        <age-rom-library [filter]="libraryFilter"
                         (romCount)="updateRomCountHint($event)"></age-rom-library>
    `,
    styles: [`
        :host {
            display: block;
            max-width: calc(240px * 4 + 16px * 5); /* rom library with 4 columns */
            margin-left: auto;
            margin-right: auto;
        }

        .hidden {
            /* let the toolbar have the correct height when no input element is visible */
            width: 0;
        }

        mat-toolbar {
            position: sticky;
            top: 0;
            min-height: min-content;
            z-index: 1;
        }

        .toolbar-contents {
            flex: 1000 1;
            max-width: ${MOBILE_WIDTH_PX}px;
            display: flex;
        }

        form {
            flex: 1 1;
            padding-left: 16px;
        }

        mat-form-field {
            width: 100%;
        }

        age-rom-library {
            margin-top: 1em;
            margin-bottom: 4em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AgeOpenRomComponent {

    readonly viewMode$: Observable<IToolbarViewMode>;
    readonly ToolbarMode = ToolbarMode;

    @Input() libraryFilter?: string;
    @Input() romFileUrl?: string;

    private readonly _toolbarModeSubject = new BehaviorSubject<ToolbarMode>(ToolbarMode.DEFAULT);
    private _romCountHint = '0 roms';

    constructor(readonly navigationService: AgeNavigationService,
                readonly icons: AgeIconsService,
                private readonly _romFileService: AgeRomFileService,
                breakpointObserverService: AgeBreakpointObserverService) {

        this.viewMode$ = combineLatest([
            viewMode$(breakpointObserverService),
            this._toolbarModeSubject.asObservable(),
        ]).pipe(
            map(values => {
                const viewMode = values[0];
                const toolbarMode = values[1];

                const roomForLeftAngle = toolbarMode === ToolbarMode.DEFAULT || !viewMode.mobileWidth;

                return {
                    ...viewMode,
                    showLeftAngle: viewMode.mobileView && roomForLeftAngle,
                    toolbar: toolbarMode,
                };
            }),
        );
    }


    get romCountHint(): string {
        return this._romCountHint;
    }

    setToolbarMode(toolbarMode: ToolbarMode): void {
        this.libraryFilter = '';
        this.romFileUrl = '';
        this._toolbarModeSubject.next(toolbarMode);
    }

    openRomFileUrl(): void {
        let romFileUrl = this.romFileUrl;
        if (romFileUrl) {
            if (!romFileUrl.startsWith('http://') && !romFileUrl.startsWith('https://')) {
                romFileUrl = `http://${romFileUrl}`;
            }
            this.navigationService.navigateToRomUrl(romFileUrl);
        }
    }

    openLocalRom(localRom: IAgeLocalRomFile, viewMode: IAgeViewMode): void {
        this._romFileService.openRomFile(localRom);
        // navigate to the emulation, if it's not yet visible
        if (viewMode.mobileView) {
            this.navigationService.navigateToRoot();
        }
    }

    updateRomCountHint(romCount: IAgeRomCount): void {
        this._romCountHint = `${romCount.currentRoms} of ${romCount.totalRoms} roms`;
    }
}


interface IToolbarViewMode extends IAgeViewMode {
    readonly showLeftAngle: boolean;
    readonly toolbar: ToolbarMode;
}

enum ToolbarMode {
    DEFAULT = 'DEFAULT',
    SEARCH_LIBRARY = 'SEARCH_LIBRARY',
    ENTER_ROM_FILE_URL = 'ENTER_ROM_FILE_URL',
}
