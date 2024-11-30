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

即使程序仅支持 IMM，也可以通过 IMM 访问基于 TSF 的输入法（但部分 IMM 功能无效，如：ImmGetIMEFileName）。

如果你的程序需要丰富的文本输入体验（如：Word），那么应该选择基于 TSF 的 IME-aware 程序。**本示例帮不到你**。

如果你的程序只需要基本的文本输入体验（如：记事本、画图），那么选择基于 IMM 的 IME-aware 程序可以让编程更加简单。

本示例是根据官方的 **immpad-level3-step3** 示例修改而来，并且去除了官方示例中的一些几乎用不到的 IMM 功能，让程序以最简单的方式实现了对输入法的支持。

在实现对输入法的支持时，强烈建议先阅读：[Windows IME-aware 程序设计指南](./docs/zh_Hans/Index.md)

特别是第一条！特别是第一条！特别是第一条！

否则会给使用输入法的用户带来非常糟糕的使用体验！！！

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

TODO.

### Build

TODO.

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
