# Windows IME-aware application example

[简体中文](#介绍) / [English](#introduce)

## 介绍

本仓库包含：

- 一个 Windows IME-aware 程序示例。

- 一份本人总结的 [Windows IME-aware 程序设计指南](./docs/zh_Hans/Index.md)。

下面是微软的官方示例链接：

[Win7Samples/winui/input/tsf/tsfapps](https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/input/tsf/tsfapps)

IME 是一个辅助用户输入中文、日文、朝鲜文等复杂文字的程序，微软提供的全部 IME 可以在这个页面中查看到：

[Input Method Editors (IME)](https://learn.microsoft.com/en-us/globalization/input/input-method-editors)

此外，也存在第三方 IME，如：搜狗拼音输入法、谷歌日语输入法。

Windows 上的输入法框架分为 IMM 和 TSF 两种。

因此分为基于 IMM 的输入法和基于 TSF 的输入法，也分为基于 IMM 的 IME-aware 程序和基于 TSF 的 IME-aware 程序。

现在的输入法基本上是基于 TSF 而不是 IMM，但是 TSF 兼容 IMM，使得基于 TSF 的输入法可以工作在基于 IMM 或基于 TSF 的 IME-aware 程序中。

即使程序仅支持 IMM，也可以通过 IMM 访问基于 TSF 的输入法（但部分 IMM 功能无效，如：`ImmGetIMEFileName`）。

如果你的程序需要提供丰富的文本输入体验（如：Word），那么应该选择基于 TSF 的 IME-aware 程序。**本示例帮不到你**。

如果你的程序只需要提供基本的文本输入体验（如：记事本、画图），那么选择基于 IMM 的 IME-aware 程序可以让编程更加简单。

本示例是根据官方的 **immpad-level3-step3** 示例修改而来，并且去除了官方示例中的一些几乎用不到的 IMM 功能，让程序以最简单的方式实现了对输入法的支持。

在为你的程序实现对输入法的支持时，强烈建议先阅读：[Windows IME-aware 程序设计指南](./docs/zh_Hans/Index.md)

特别是第一条！如果你不遵守第一条，则会给使用输入法的用户带来非常糟糕的使用体验！

### 生成

1. 安装 [Visual Studio 2022 Build Tools](https://learn.microsoft.com/zh-cn/visualstudio/install/use-command-line-parameters-to-install-visual-studio?view=vs-2022)

   - 安装 **使用 C++ 的桌面开发** 工作负荷

   - 安装 **Windows SDK**（上述的工作负荷已包含）

   开发者所用版本：

   - Visual Studio 2022 Build Tools 17.9.6

   - MSVC 14.39.33519

   - Windows SDK 10.0.22621.0

   > 如果使用不同的版本，请适当修改 **ImmPad.vcxproj** 中的 `PlatformToolset` 和 `WindowsTargetPlatformVersion` 属性。

2. 运行 **make-build.bat**。编译完成后请通过 **make-run.bat** 来运行程序，这样就可以在控制台中看到非 ASCII 字符。

---

## Introduce

This repository contains:

- An example of a Windows IME-aware application.

- A guide summarized by myself: [Windows IME-aware Application Design Guide](./docs/en/Index.md)

Here is the official example link from Microsoft:

[Win7Samples/winui/input/tsf/tsfapps](https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/winui/input/tsf/tsfapps)

IME is a program that assists users in inputting complex characters such as Chinese, Japanese, Korean, etc. All IMEs provided by Microsoft can be viewed on this page:

[Input Method Editors (IME)](https://learn.microsoft.com/en-us/globalization/input/input-method-editors)

In addition, there are also third-party IMEs such as [Sogou Pinyin IME](https://shurufa.sogou.com/windows) and [Google Japanese IME](https://www.google.co.jp/ime/).

The input method framework on Windows is divided into two types: IMM and TSF.

Therefore, it is divided into IMM based IME and TSF based IME, as well as IMM based IME-aware application and TSF based IME-aware application.

The current IME are mostly based on TSF rather than IMM, but TSF is compatible with IMM, allowing TSF based IME to work in IMM based or TSF based IME-aware application.

Even if the program only supports IMM, it is still possible to access TSF based IME through IMM (although some IMM features are invalid, such as `ImmGetIMEFileName`）。

If your program needs to provide a rich text input experience (such as Microsoft Word), so we should choose an IME-aware application based on TSF, and **this example cannot help you**.

If your program only needs to provide a basic text input experience (such as Windows Notepad, MS Paint), then choosing an IMM based IME-aware application can make programming easier.

This example is modified based on the official **immpad-level3-step3** example, and removes some almost unused IMM features from the official example, allowing the program to implement support for IME in the simplest way possible.

When implementing support for IME in your program, it is strongly recommended to first read [Windows IME-aware Application Design Guide](./docs/en/Index.md).

Especially the first one! If you do not comply with the first rule, it will bring a very bad user experience to those who use input methods!

### Build

1. Install [Visual Studio 2022 Build Tools](https://learn.microsoft.com/en-us/visualstudio/install/use-command-line-parameters-to-install-visual-studio?view=vs-2022)

   - Install **Desktop development with C++** workload

   - Install **Windows SDK** (The above workload already includes)

   Version used by myself：

   - Visual Studio 2022 Build Tools 17.9.6

   - MSVC 14.39.33519

   - Windows SDK 10.0.22621.0

   > If using different versions, please modify the `PlatformToolset` and `WindowsTargetPlatformVersion` properties in **ImmPad.vcxproj** appropriately.

2. Run **make-build.bat**. After compilation, please run the program through **make-run.bat** so that non ASCII characters can be seen in the console.

# 参考资料 (Reference)

[imm.h header](https://learn.microsoft.com/en-us/windows/win32/api/imm/)

[Input Method Editors (IME)](https://learn.microsoft.com/en-us/globalization/input/input-method-editors)

[Simplified Chinese IMEs](https://learn.microsoft.com/en-us/globalization/input/simplified-chinese-ime)

[Microsoft Simplified Chinese IME](https://support.microsoft.com/en-us/windows/microsoft-simplified-chinese-ime-9b962a3b-2fa4-4f37-811c-b1886320dd72)

[Traditional Chinese IME](https://learn.microsoft.com/en-us/globalization/input/traditional-chinese-ime)

[Microsoft Traditional Chinese IME](https://support.microsoft.com/en-us/windows/microsoft-traditional-chinese-ime-ef596ca5-aff7-4272-b34b-0ac7c2631a38)

[Japanese IME](https://learn.microsoft.com/en-us/globalization/input/japanese-ime)

[Microsoft Japanese IME](https://support.microsoft.com/en-us/windows/microsoft-japanese-ime-da40471d-6b91-4042-ae8b-713a96476916)

[Korean IME](https://learn.microsoft.com/en-us/globalization/input/korean-ime)
