import {
    AfterViewInit,
    ChangeDetectionStrategy,
    ChangeDetectorRef,
    Component,
    ElementRef,
    HostListener,
    Inject,
    Input,
    OnDestroy,
    OnInit,
    ViewChild
} from '@angular/core';
import {AgeEmulation, AgeGbEmulation} from './age-emulation';
import {AgeGbKeyMap} from './age-emulator-keymap';


@Component({
    selector: 'age-emulator',
    template: `
        <canvas #emulatorCanvas width="160" height="144"></canvas>
    `,
    styles: [`
        :host {
            width: 100%;
            height: 100%;
            background-color: aquamarine;
        }
    `],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class AgeEmulatorComponent implements OnInit, OnDestroy, AfterViewInit {

    private readonly _keyMap = new AgeGbKeyMap();

    @ViewChild('emulatorCanvas')
    private _emulatorCanvas: ElementRef;
    private _canvas2dCtx: CanvasRenderingContext2D | undefined;

    private _emGbModule: EmGbModule | undefined;
    private _romFileContents: ArrayBuffer | undefined;
    private _emulation: AgeEmulation | undefined;
    private _timerHandle: number | undefined;

    constructor(@Inject(ChangeDetectorRef) private _changeDetector: ChangeDetectorRef) {
    }


    @HostListener('document:keydown', ['$event'])
    handleKeyDown(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulation && gbButton) {
            this._emulation.buttonDown(gbButton);
            event.preventDefault();
        }
    }

    @HostListener('document:keyup', ['$event'])
    handleKeyUp(event: KeyboardEvent) {
        const gbButton = this._keyMap.getButtonForKey(event.key);

        if (this._emulation && gbButton) {
            this._emulation.buttonUp(gbButton);
            event.preventDefault();
        }
    }


    ngOnInit(): void {
        this._timerHandle = window.setInterval(
            this.emulate.bind(this),
            10
        );
    }

    ngOnDestroy(): void {
        if (this._timerHandle) {
            window.clearInterval(this._timerHandle);
            this._timerHandle = undefined;
        }
    }

    ngAfterViewInit(): void {
        const canvas = this._emulatorCanvas.nativeElement;
        this._canvas2dCtx = canvas.getContext('2d');
    }


    @Input()
    set emGbModule(emGbModule: EmGbModule) {
        this._emGbModule = emGbModule;
        this.newEmulation();
    }

    @Input()
    set romFileContents(romFileContents: ArrayBuffer) {
        this._romFileContents = romFileContents;
        this.newEmulation();
    }


    private newEmulation(): void {
        if (this._emGbModule && this._romFileContents) {
            this._emulation = new AgeGbEmulation(this._emGbModule, this._romFileContents);
        }
    }

    private emulate(): void {
        if (this._emulation) {
            const newFrame = this._emulation.emulate();

            // update the canvas if there is a new frame to display
            if (newFrame && this._canvas2dCtx) {
                const bytes = this._emulation.getScreenBuffer();
                const imageData = new ImageData(bytes, this._emulation.getScreenWidth(), this._emulation.getScreenHeight());
                this._canvas2dCtx.putImageData(imageData, 0, 0);
                this._changeDetector.detectChanges();
            }
        }
    }
}
