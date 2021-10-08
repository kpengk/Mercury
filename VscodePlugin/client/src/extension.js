"use strict";
/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */
Object.defineProperty(exports, "__esModule", { value: true });
exports.deactivate = exports.activate = void 0;
const path = require("path");
const vscode_1 = require("vscode");
const node_1 = require("vscode-languageclient/node");
let client;
function activate(context) {
    const serverModule = context.asAbsolutePath(path.join('server', 'out', 'server.js'));
    // 为服务器提供debug选项
    // --inspect=6009: 运行在Node's Inspector mode，这样VS Code就能调试服务器了
    const debugOptions = { execArgv: ['--nolazy', '--inspect=6009'] };
    // 如果扩展在调试模式下启动，则使用调试服务器选项，否则使用运行选项
    const serverOptions = {
        run: { module: serverModule, transport: node_1.TransportKind.ipc },
        debug: {
            module: serverModule,
            transport: node_1.TransportKind.ipc,
            options: debugOptions
        }
    };
    // 控制语言客户端的选项
    const clientOptions = {
        // 注册纯文本服务器
        documentSelector: [{ scheme: 'file', language: 'plaintext' }],
        synchronize: {
            // 当文件变动为'.clientrc'时，通知服务器
            fileEvents: vscode_1.workspace.createFileSystemWatcher('**/.clientrc')
        }
    };
    // 创建语言客户端并启动
    client = new node_1.LanguageClient('languageServerExample', 'Language Server Example', serverOptions, clientOptions);
    // 启动客户端，这也同时启动了服务器
    client.start();
}
exports.activate = activate;
function deactivate() {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
exports.deactivate = deactivate;
//# sourceMappingURL=extension.js.map