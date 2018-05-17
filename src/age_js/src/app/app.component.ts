import {AfterViewInit, ChangeDetectionStrategy, Component, ElementRef, HostListener, Input, ViewChild} from '@angular/core';
import {VERSION_INFO} from '../environments/version';
import {AgeRect} from './modules/common/age-rect';
import {AgeEmulationPackage} from './modules/common/age-emulation-package';
import {AgeRomFileToLoad} from './modules/common/age-rom-file-to-load';


@Component({
    selector: 'age-app-root',
    template: `
        <div class="container">

            <div>
                Welcome to the AGE-JS prototype
            </div>

            <div>
                <div><b>Build Details</b></div>
                <div>Commit Hash: {{versionHash}}</div>
                <div>Committed on: {{versionDate | date:'y-MM-dd HH:mm:ss'}}</div>
            </div>

            <div>
                <table>
                    <tr>
                        <th colspan="4">Key Mappings</th>
                    </tr>
                    <tr>
                        <th>Gameboy</th>
                        <th>Keyboard</th>
                        <th>Gameboy</th>
                        <th>Keyboard</th>
                    </tr>
                    <tr>
                        <td>Up</td>
                        <td>Up Arrow</td>
                        <td>A</td>
                        <td>A</td>
                    </tr>
                    <tr>
                        <td>Down</td>
                        <td>Down Arrow</td>
                        <td>B</td>
                        <td>S</td>
                    </tr>
                    <tr>
                        <td>Left</td>
                        <td>Left Arrow</td>
                        <td>Start</td>
                        <td>Space</td>
                    </tr>
                    <tr>
                        <td>Right</td>
                        <td>Right Arrow</td>
                        <td>Select</td>
                        <td>Enter</td>
                    </tr>
                </table>
            </div>

            <div>
                <age-file-selector (fileSelected)="selectFileToLoad($event)"></age-file-selector>
            </div>

            <div #emulatorContainer>
                <age-loader [romFileToLoad]="romFileToLoad"
                            (loadingComplete)="loadingComplete($event)"></age-loader>

                <age-emulator [emulationPackage]="emulationPackage"
                              [viewport]="viewport"></age-emulator>
            </div>

        </div>
    `,
    styles: [`
        .container {
            display: flex;
            height: 100%;
            flex-direction: column;
            overflow: auto;
        }

        .container > div {
            margin-bottom: 2em;
            text-align: center;
            font-size: medium;
        }

        .container > div:nth-child(1) {
            font-size: x-large;
            font-weight: bold;
        }

        .container > div:nth-child(2) {
            font-size: smaller;
        }

        .container > div:nth-child(3) {
            font-size: smaller;
        }

        .container > div:nth-child(5) {
            flex: 1;
            min-height: 200px;
            min-width: 200px;
            overflow: hidden;
        }

        table {
            margin: auto;
        }

        table th {
            font-style: italic;
            font-weight: normal;
            text-align: right;
        }

        table th:nth-child(even) {
            text-align: left;
            padding-left: .5em;
        }

        table tr:nth-child(1) th {
            font-style: normal;
            font-weight: bold;
            text-align: center;
        }

        table td {
            text-align: right;
        }

        table td:nth-child(even) {
            text-align: left;
            padding-left: .5em;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AppComponent implements AfterViewInit {

    @Input() romFileToLoad?: AgeRomFileToLoad;

    @ViewChild('emulatorContainer')
    private _emulatorContainer: ElementRef;
    private _emulationPackage?: AgeEmulationPackage;
    private _viewport = new AgeRect(1, 1);

    private _versionInfo = VERSION_INFO;


    ngAfterViewInit(): void {
        this.emulatorContainerResize();
    }


    get versionDate(): string {
        return this._versionInfo.date;
    }

    get versionHash(): string {
        return this._versionInfo.hash;
    }

    get emulationPackage(): AgeEmulationPackage | undefined {
        return this._emulationPackage;
    }

    get viewport(): AgeRect {
        return this._viewport;
    }


    selectFileToLoad(fileToLoad: AgeRomFileToLoad): void {
        this.romFileToLoad = fileToLoad;
        this._emulationPackage = undefined;
    }

    loadingComplete(emulationPackage: AgeEmulationPackage) {
        this._emulationPackage = emulationPackage;
    }

    @HostListener('window:resize')
    emulatorContainerResize(): void {
        this._viewport = new AgeRect(
            this._emulatorContainer.nativeElement.offsetWidth,
            this._emulatorContainer.nativeElement.offsetHeight
        );
    }
}