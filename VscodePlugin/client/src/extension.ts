/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */

import * as path from 'path';
import { window, Terminal, commands, workspace, ExtensionContext } from 'vscode';

import {
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;
let terminal_: Terminal | undefined;

export function activate(context: ExtensionContext) {
	// register command IDL To CPP
	let disposable = commands.registerCommand('g6-abi-generator.idltocpp', () => {
		let doc =  window.activeTextEditor!.document;
		let path = doc.uri.fsPath;
		
		// interface file path
		let dirPath: string = path.substring(0, path.lastIndexOf('\\'));
		if (dirPath.length === 0) {
			dirPath = path.substring(0, path.lastIndexOf('/'));
		}

		let outPath: string = workspace.getConfiguration().get("g6-abi-generator.outputPath");
		if (outPath === null) {
			outPath = "IDL-Out";
		}
		let binPath: string = workspace.getConfiguration().get("g6-abi-generator.generatorPath");
		let cmdStr = `interface_generator_app.exe -i ${path} -d ${dirPath} -o ${outPath}`;
		if (binPath !== null) {
			if (!binPath.endsWith('\\') && !binPath.endsWith('/')) {
				binPath += '/';
			}
			cmdStr = `${binPath}interface_generator_app.exe -i ${path} -d ${dirPath} -o ${outPath}`;
		}

		if (terminal_ === undefined) {
            terminal_ = window.createTerminal("IdlToCpp");
        }
		terminal_.show(false);
		terminal_.sendText(cmdStr);
	});
	context.subscriptions.push(disposable);


	// 服务器由node实现
	const serverModule = context.asAbsolutePath(
		path.join('server', 'out', 'server.js')
	);
	// 为服务器提供debug选项
    // --inspect=6009: 运行在Node's Inspector mode，这样VS Code就能调试服务器了
	const debugOptions = { execArgv: ['--nolazy', '--inspect=6009'] };

	// 如果扩展在调试模式下启动，则使用调试服务器选项，否则使用运行选项
	const serverOptions: ServerOptions = {
		run: { module: serverModule, transport: TransportKind.ipc },
		debug: {
			module: serverModule,
			transport: TransportKind.ipc,
			options: debugOptions
		}
	};

	// 控制语言客户端的选项
	const clientOptions: LanguageClientOptions = {
		// 注册G6IDL服务器
		documentSelector: [{ scheme: 'file', language: 'G6IDL' }],
		synchronize: {
			// 当文件变动为'.g6idl'时，通知服务器
			fileEvents: workspace.createFileSystemWatcher('**/.g6idl')
		}
	};

	// 创建语言客户端并启动
	client = new LanguageClient(
		'g6IdlLanguageServer',
		'G6 IDL Language Server',
		serverOptions,
		clientOptions
	);

	// 启动客户端，这也同时启动了服务器
	client.start();
}

export function deactivate(): Thenable<void> | undefined {
	if (!client) {
		return undefined;
	}
	return client.stop();
}

window.onDidCloseTerminal((terminal) => {
	if (terminal.name === "IdlToCpp") {
		terminal_ = undefined;
		window.showInformationMessage("Did Close Terminal");
	}
});
