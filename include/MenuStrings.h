#pragma once

constexpr FakeString<3> BorderTop =         fixString(U"┌─┐");
constexpr FakeString<3> BorderTopStart =    fixString(U"┌─┬");
constexpr FakeString<3> BorderTopMid =      fixString(U"┬─┬");
constexpr FakeString<3> BorderTopEnd =      fixString(U"┬─┐");

constexpr FakeString<3> BorderMid =         fixString(U"│ │");

constexpr FakeString<3> BorderSep =         fixString(U"├─┤");
constexpr FakeString<3> BorderSepStart =    fixString(U"├─┼");
constexpr FakeString<3> BorderSepMid =      fixString(U"┼─┼");
constexpr FakeString<3> BorderSepEnd =      fixString(U"┼─┤");

constexpr FakeString<3> BorderBottom =      fixString(U"└─┘");
constexpr FakeString<3> BorderBottomStart = fixString(U"└─┴");
constexpr FakeString<3> BorderBottomMid =   fixString(U"┴─┴");
constexpr FakeString<3> BorderBottomEnd =   fixString(U"┴─┘");
//constexpr FakeString<5> BorderSep =    fixString(U"├┴┼┬┤");

constexpr FakeString<3> DoubleBorderTop =           fixString(U"╔═╗");
constexpr FakeString<3> DoubleBorderTopStart =      fixString(U"╔═╦");
constexpr FakeString<3> DoubleBorderTopMid =        fixString(U"╦═╦");
constexpr FakeString<3> DoubleBorderTopEnd =        fixString(U"╦═╗");

constexpr FakeString<3> DoubleBorderMid =           fixString(U"║ ║");

constexpr FakeString<3> DoubleBorderSep =           fixString(U"╠═╣");
constexpr FakeString<3> DoubleBorderSepStart =      fixString(U"╠═╬");
constexpr FakeString<3> DoubleBorderSepMid =        fixString(U"╬═╬");
constexpr FakeString<3> DoubleBorderSepEnd =        fixString(U"╬═╣");

constexpr FakeString<3> DoubleBorderBottom =        fixString(U"╚═╝");
constexpr FakeString<3> DoubleBorderBottomStart =   fixString(U"╚═╩");
constexpr FakeString<3> DoubleBorderBottomMid =     fixString(U"╩═╩");
constexpr FakeString<3> DoubleBorderBottomEnd =     fixString(U"╩═╝");
//constexpr FakeString<5> DoubleBorderSep =    fixString(U"╠╩╬╦╣");

constexpr FakeString<42> Title[] {
    fixString(U"┌─────┐               ┌─────┐             "),
    fixString(U"│ ┌─┐ │  ┌─┐┌─────┐   │ ┌─┐ │┌─────┬─────┐"),
    fixString(U"│ └─┘ └┐ │ │└─┐ ┌─┘   │ └─┘ └┤ ┌─┐ ├─┐ ┌─┘"),
    fixString(U"│ ┌──┐ │ │ │  │ │     │ ┌──┐ │ │ │ │ │ │  "),
    fixString(U"│ └──┘ │ │ │  │ │     │ │  │ │ └─┘ │ │ │  "),
    fixString(U"└──────┘ └─┘  └─┘     └─┘  └─┴─────┘ └─┘  "),
};

constexpr FakeString<47> TitleSettings[] {
    fixString(U"┌──────┐                                       "),
    fixString(U"│ ┌────┼────┬─────┬─────┐┌─┐┌─────┬─────┬─────┐"),
    fixString(U"│ └────┤  ─┬┴─┐ ┌─┴─┐ ┌─┘│ ││ ┌─┐ │ ┌───┤ ┌───┘"),
    fixString(U"└────┐ │ ┌─┘  │ │   │ │  │ ││ │ │ │ │┌──┤ └───┐"),
    fixString(U"┌────┘ │ └──┐ │ │   │ │  │ ││ │ │ │ └┴─ ├───  │"),
    fixString(U"└──────┴────┘ └─┘   └─┘  └─┘└─┘ └─┴─────┴─────┘"),
};

constexpr FakeString SettingsTop =      fixString(U"┌──────────────┬───────────────────────────────────────────────────────────┐");
constexpr FakeString SettingsMid =      fixString(U"│              │                                                           │");
constexpr FakeString SettingsSep =      fixString(U"├──────────────┼───────────────────────────────────────────────────────────┤");
constexpr FakeString SettingsBottom =   fixString(U"└──────────────┴───────────────────────────────────────────────────────────┘");


