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
import {Injectable, OnDestroy} from '@angular/core';
import {TAgeRomFile} from 'age-lib';
import {BehaviorSubject, Observable} from 'rxjs';


@Injectable()
export class AgeRomFileService implements OnDestroy {

    // to counter any race conditions during component initialization the
    // current rom file is cached by the subject
    private readonly _openRomFileSubject = new BehaviorSubject<TAgeRomFile | undefined>(undefined);
    private readonly _openRomFile$ = this._openRomFileSubject.asObservable();

    ngOnDestroy(): void {
        this._openRomFileSubject.complete();
    }

    get openRomFile$(): Observable<TAgeRomFile | undefined> {
        return this._openRomFile$;
    }

    openRomFile(romFile: TAgeRomFile | undefined): void {
        this._openRomFileSubject.next(romFile);
    }
}
