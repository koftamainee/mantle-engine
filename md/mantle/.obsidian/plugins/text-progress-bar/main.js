'use strict';

var obsidian = require('obsidian');

/******************************************************************************
Copyright (c) Microsoft Corporation.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
***************************************************************************** */

function __awaiter(thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
}

const emojiRegex = /\p{Emoji}/u;
class ProgressBar {
    constructor(label, done, total) {
        this.label = label;
        this.done = done;
        this.total = total;
    }
    get fill() {
        return this._fill;
    }
    set fill(value) {
        this._fill = value;
    }
    get length() {
        return this._length > 0 ? this._length : this.total;
    }
    set length(value) {
        this._length = value;
    }
    get transition() {
        return this._transition.join(",");
    }
    set transition(value) {
        this._transition = value.split(",");
    }
    renderTransition() {
        return this._transition[Math.floor(this.getRemainder() * this._transition.length)];
    }
    getDonePercent() {
        return (this.done / this.total) * 100;
    }
    getRemainder() {
        return ((this.done / this.total) * this.length) % 1;
    }
    getDoneParts() {
        return Math.min(Math.floor((this.done / this.total) * this.length), this.length);
    }
    /**
     *
     * Renders the output of the bar.
     *
     * @param container HTML Element which will contain the bar.
     * @returns An HTML element of the bar.
     */
    renderBar(container) {
        // Add a text label in front of the bar.
        if (!this.labelHide) {
            container.createEl("span", {
                text: this.label,
                cls: "label"
            });
        }
        const bar = document.createElement("span");
        bar.className = "bar";
        if (this.fill.match(emojiRegex)) {
            bar.className += " has-emoji";
        }
        const prefix = bar.createEl("span");
        prefix.innerText = this.prefix;
        const complete = this.getDoneParts();
        for (let index = 0; index < this.length; index++) {
            const bit = bar.createEl("span");
            if (index < complete || complete === this.length) {
                bit.className = "filled";
                bit.innerText = this.fill;
            }
            else {
                bit.className = "empty";
                bit.innerText = (complete === index && this.getRemainder()) ? this.renderTransition() : this.empty;
            }
        }
        const suffix = bar.createEl("span");
        suffix.innerText = this.suffix;
        return bar;
    }
}

const labelRegex = /(?<label>.+):\s*(?<done>(\d+\.?\d*|\d*\.?\d+))\/(?<total>(\d+\.?\d*|\d*\.?\d+))/;
const DEFAULT_SETTINGS = {
    transition: '🌘,🌗,🌔',
    fill: '🌕',
    empty: '🌑',
    prefix: '',
    suffix: '',
    length: 0,
    labelHide: false
};
class TextProgress extends obsidian.Plugin {
    parseLabel(label) {
        const matchResult = label.match(labelRegex);
        if (matchResult) {
            const groups = matchResult.groups;
            if (groups && groups.label) {
                return {
                    'label': groups.label,
                    'done': +groups.done,
                    'total': +groups.total
                };
            }
        }
        return false;
    }
    parseSource(rows) {
        // Other rows are settings for the bar.
        // Formatted like "setting:value"
        const settings = {
            "transition": this.settings.transition,
            "prefix": this.settings.prefix,
            "suffix": this.settings.suffix,
            "fill": this.settings.fill,
            "empty": this.settings.empty,
            "labelHide": this.settings.labelHide,
            "length": this.settings.length,
        };
        // For each setting in the progress bar, merge with
        // current settings.
        // We split on the first ':' as there could be ':' in
        // the values.
        for (const row in rows) {
            const [first, ...rest] = rows[row].split(':');
            const restJoined = rest.join(":");
            settings[first] = restJoined;
        }
        return settings;
    }
    onload() {
        return __awaiter(this, void 0, void 0, function* () {
            yield this.loadSettings();
            this.registerMarkdownCodeBlockProcessor("text-progress-bar", (source, el, ctx) => __awaiter(this, void 0, void 0, function* () {
                const rows = source.trim().split("\n");
                const labels = [];
                // Get all labels that match regex.
                for (const row in rows) {
                    const label = this.parseLabel(rows[row]);
                    if (label) {
                        labels.push(label);
                    }
                }
                if (labels.length) {
                    const container = el.createEl("section");
                    container.className = "text-progress-bar";
                    for (const parsedLabel of labels) {
                        const wrapper = container.createEl("div");
                        if (typeof parsedLabel !== 'boolean') {
                            const bar = new ProgressBar(parsedLabel.label, parsedLabel.done, parsedLabel.total);
                            const settings = this.parseSource(rows);
                            bar.transition = settings.transition;
                            bar.prefix = settings.prefix;
                            bar.suffix = settings.suffix;
                            bar.fill = settings.fill;
                            bar.length = settings.length;
                            bar.empty = settings.empty;
                            bar.labelHide = String(settings.labelHide).toLowerCase() === "true";
                            wrapper.appendChild(bar.renderBar(wrapper));
                        }
                    }
                }
                else {
                    new obsidian.Notice('No progress bars found.');
                }
            }));
            // Adds a settings tab so the user can configure various aspects of the plugin
            this.addSettingTab(new TextProgressSettingsTab(this.app, this));
        });
    }
    loadSettings() {
        return __awaiter(this, void 0, void 0, function* () {
            this.settings = Object.assign({}, DEFAULT_SETTINGS, yield this.loadData());
        });
    }
    saveSettings() {
        return __awaiter(this, void 0, void 0, function* () {
            yield this.saveData(this.settings);
        });
    }
}
class TextProgressSettingsTab extends obsidian.PluginSettingTab {
    constructor(app, plugin) {
        super(app, plugin);
        this.plugin = plugin;
    }
    display() {
        const { containerEl } = this;
        containerEl.empty();
        containerEl.createEl('h2', { text: 'Settings' });
        new obsidian.Setting(containerEl)
            .setName('Fill')
            .setDesc('A character representing the filled part of your bar.')
            .addText(text => text
            .setPlaceholder('🌕')
            .setValue(this.plugin.settings.fill)
            .onChange((value) => __awaiter(this, void 0, void 0, function* () {
            this.plugin.settings.fill = value;
            yield this.plugin.saveSettings();
        })));
        new obsidian.Setting(containerEl)
            .setName('Empty')
            .setDesc('A character representing the un-filled part of your bar.')
            .addText(text => text
            .setPlaceholder('🌑')
            .setValue(this.plugin.settings.empty)
            .onChange((value) => __awaiter(this, void 0, void 0, function* () {
            this.plugin.settings.empty = value;
            yield this.plugin.saveSettings();
        })));
        new obsidian.Setting(containerEl)
            .setName('Transition')
            .setDesc('Enter one or multiple characters (separated by comma) to have a transition between filled and empty.  Eg 🌒,🌓,🌔')
            .addText(text => text
            .setValue(this.plugin.settings.transition)
            .onChange((value) => __awaiter(this, void 0, void 0, function* () {
            this.plugin.settings.transition = value;
            yield this.plugin.saveSettings();
        })));
        new obsidian.Setting(containerEl)
            .setName('Prefix')
            .setDesc('Enter a prefix to start your bar with.  Eg "["')
            .addText(text => text
            .setValue(this.plugin.settings.prefix)
            .onChange((value) => __awaiter(this, void 0, void 0, function* () {
            this.plugin.settings.prefix = value;
            yield this.plugin.saveSettings();
        })));
        new obsidian.Setting(containerEl)
            .setName('Suffix')
            .setDesc('Enter a suffix to end your bar with.  Eg "]"')
            .addText(text => text
            .setValue(this.plugin.settings.suffix)
            .onChange((value) => __awaiter(this, void 0, void 0, function* () {
            this.plugin.settings.suffix = value;
            yield this.plugin.saveSettings();
        })));
        new obsidian.Setting(containerEl)
            .setName('Hide labels')
            .setDesc('If labels should be hidden or shown')
            .addToggle((toggle) => {
            toggle.setValue(this.plugin.settings.labelHide).onChange((newState) => __awaiter(this, void 0, void 0, function* () {
                this.plugin.settings.labelHide = newState;
                yield this.plugin.saveSettings();
            }));
        });
    }
}

module.exports = TextProgress;


/* nosourcemap */