"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */
const node_1 = require("vscode-languageserver/node");
const vscode_languageserver_textdocument_1 = require("vscode-languageserver-textdocument");
// 创建一个服务器连接。使用Node的IPC作为传输方式。 也包含所有的预览、建议等LSP特性
const connection = (0, node_1.createConnection)(node_1.ProposedFeatures.all);
// 创建一个简单的文本管理器。文本管理器只支持全文本同步。
const documents = new node_1.TextDocuments(vscode_languageserver_textdocument_1.TextDocument);
let hasConfigurationCapability = false;
let hasWorkspaceFolderCapability = false;
let hasDiagnosticRelatedInformationCapability = false;
connection.onInitialize((params) => {
    const capabilities = params.capabilities;
    // 客户端是否支持`workspace/configuration`请求?
    // 如果不是的话，降级到使用全局设置
    hasConfigurationCapability = !!(capabilities.workspace && !!capabilities.workspace.configuration);
    hasWorkspaceFolderCapability = !!(capabilities.workspace && !!capabilities.workspace.workspaceFolders);
    hasDiagnosticRelatedInformationCapability = !!(capabilities.textDocument &&
        capabilities.textDocument.publishDiagnostics &&
        capabilities.textDocument.publishDiagnostics.relatedInformation);
    const result = {
        capabilities: {
            textDocumentSync: node_1.TextDocumentSyncKind.Incremental,
            // 告诉客户端此服务器支持代码完成。
            completionProvider: {
                resolveProvider: true
            }
        }
    };
    if (hasWorkspaceFolderCapability) {
        result.capabilities.workspace = {
            workspaceFolders: {
                supported: true
            }
        };
    }
    return result;
});
connection.onInitialized(() => {
    if (hasConfigurationCapability) {
        // 注册所有配置更改
        connection.client.register(node_1.DidChangeConfigurationNotification.type, undefined);
    }
    if (hasWorkspaceFolderCapability) {
        connection.workspace.onDidChangeWorkspaceFolders(_event => {
            connection.console.log('Workspace folder change event received.');
        });
    }
});
// 当客户端不支持`workspace/configuration`请求时使用的全局设置。
// 请注意，当将此服务器与本示例中提供的客户端一起使用时，情况并非如此，但其他客户端可能会出现这种情况。
const defaultSettings = { maxNumberOfProblems: 1000 };
let globalSettings = defaultSettings;
// 缓存所有打开文档的设置
const documentSettings = new Map();
// 文档的文本内容发生了改变。
// 这个事件在文档第一次打开或者内容变动时才会触发。
connection.onDidChangeConfiguration(change => {
    if (hasConfigurationCapability) {
        // 重置所有缓存的文档设置
        documentSettings.clear();
    }
    else {
        globalSettings = ((change.settings.languageServerExample || defaultSettings));
    }
    // 重新验证所有打开的文本文档
    documents.all().forEach(validateTextDocument);
});
function getDocumentSettings(resource) {
    if (!hasConfigurationCapability) {
        return Promise.resolve(globalSettings);
    }
    let result = documentSettings.get(resource);
    if (!result) {
        result = connection.workspace.getConfiguration({
            scopeUri: resource,
            section: 'languageServerExample'
        });
        documentSettings.set(resource, result);
    }
    return result;
}
// 仅保留打开文档的设置
documents.onDidClose(e => {
    documentSettings.delete(e.document.uri);
});
// 文本文档的内容已更改。此事件在文本文档首次打开或其内容更改时发出。
documents.onDidChangeContent(change => {
    validateTextDocument(change.document);
});
async function validateTextDocument(textDocument) {
    // 在这个简单的示例中，我们将获得每次验证运行的设置。
    const settings = await getDocumentSettings(textDocument.uri);
    // 校验器如果检测到连续超过2个以上的大写字母则会报错
    const text = textDocument.getText();
    const pattern = /\b[A-Z]{2,}\b/g;
    let m;
    let problems = 0;
    const diagnostics = [];
    while ((m = pattern.exec(text)) && problems < settings.maxNumberOfProblems) {
        problems++;
        const diagnostic = {
            severity: node_1.DiagnosticSeverity.Warning,
            range: {
                start: textDocument.positionAt(m.index),
                end: textDocument.positionAt(m.index + m[0].length)
            },
            message: `${m[0]} is all uppercase.`,
            source: 'ex'
        };
        if (hasDiagnosticRelatedInformationCapability) {
            diagnostic.relatedInformation = [
                {
                    location: {
                        uri: textDocument.uri,
                        range: Object.assign({}, diagnostic.range)
                    },
                    message: 'Spelling matters'
                },
                {
                    location: {
                        uri: textDocument.uri,
                        range: Object.assign({}, diagnostic.range)
                    },
                    message: 'Particularly for names'
                }
            ];
        }
        diagnostics.push(diagnostic);
    }
    // 将错误处理结果发送给VS Code
    connection.sendDiagnostics({ uri: textDocument.uri, diagnostics });
}
connection.onDidChangeWatchedFiles(_change => {
    // Monitored files have change in VSCode
    connection.console.log('We received an file change event');
});
// // 这个处理函数提供了初始补全项列表
connection.onCompletion((_textDocumentPosition) => {
    // 传入的变量包含了文本请求代码补全的位置。
    // 在这个示例中我们忽略了这个信息，总是提供相同的补全选项。
    return [
        {
            label: 'TypeScript',
            kind: node_1.CompletionItemKind.Text,
            data: 1
        },
        {
            label: 'JavaScript',
            kind: node_1.CompletionItemKind.Text,
            data: 2
        }
    ];
});
// 这个函数为补全列表的选中项提供了更多信息
connection.onCompletionResolve((item) => {
    if (item.data === 1) {
        item.detail = 'TypeScript details';
        item.documentation = 'TypeScript documentation';
    }
    else if (item.data === 2) {
        item.detail = 'JavaScript details';
        item.documentation = 'JavaScript documentation';
    }
    return item;
});
// 让文档管理器监听文档的打开，变动和关闭事件。
documents.listen(connection);
// 连接后启动监听
connection.listen();
//# sourceMappingURL=server.js.map