// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE-CHROMIUM file.

#include "browser/devtools_manager_delegate.h"

#include <vector>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "common/content_client.h"
#include "content/browser/devtools/devtools_http_handler.h"
#include "content/shell/grit/shell_resources.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_frontend_host.h"
#include "content/public/browser/devtools_socket_factory.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/user_agent.h"
#include "net/base/net_errors.h"
#include "net/log/net_log_source.h"
#include "net/socket/tcp_server_socket.h"
#include "net/socket/stream_socket.h"
#include "ui/base/resource/resource_bundle.h"


namespace brightray {

namespace {

class TCPServerSocketFactory
    : public content::DevToolsSocketFactory {
 public:
  TCPServerSocketFactory(const std::string& address, int port)
      : address_(address), port_(port) {
  }

 private:
  // content::DevToolsSocketFactory.
  std::unique_ptr<net::ServerSocket> CreateForHttpServer() override {
    std::unique_ptr<net::ServerSocket> socket(
        new net::TCPServerSocket(nullptr, net::NetLogSource()));
    if (socket->ListenWithAddressAndPort(address_, port_, 10) != net::OK)
      return std::unique_ptr<net::ServerSocket>();

    return socket;
  }

  std::unique_ptr<net::ServerSocket> CreateForTethering(
    std::string* out_name) override {
    std::unique_ptr<net::ServerSocket> socket(
        new net::TCPServerSocket(nullptr, net::NetLogSource()));
    if (socket->ListenWithAddressAndPort(address_, port_, 10) != net::OK)
      return std::unique_ptr<net::ServerSocket>();

    return socket;
  }

  std::string address_;
  uint16_t port_;

  DISALLOW_COPY_AND_ASSIGN(TCPServerSocketFactory);
};

std::unique_ptr<content::DevToolsSocketFactory>
CreateSocketFactory() {
  auto& command_line = *base::CommandLine::ForCurrentProcess();
  // See if the user specified a port on the command line (useful for
  // automation). If not, use an ephemeral port by specifying 0.
  int port = 0;
  if (command_line.HasSwitch(switches::kRemoteDebuggingPort)) {
    int temp_port;
    std::string port_str =
        command_line.GetSwitchValueASCII(switches::kRemoteDebuggingPort);
    if (base::StringToInt(port_str, &temp_port) &&
        temp_port > 0 && temp_port < 65535) {
      port = temp_port;
    } else {
      DLOG(WARNING) << "Invalid http debugger port number " << temp_port;
    }
  }
  return std::unique_ptr<content::DevToolsSocketFactory>(
          new TCPServerSocketFactory("127.0.0.1", port));
}

}  // namespace

// DevToolsManagerDelegate ---------------------------------------------------

// static
void DevToolsManagerDelegate::StartHttpHandler() {
  std::string frontend_url;
  content::DevToolsAgentHost::StartRemoteDebuggingServer(
      CreateSocketFactory(),
      frontend_url,
      base::FilePath(),
      base::FilePath());
}

// static
void DevToolsManagerDelegate::StopHttpHandler() {
  content::DevToolsAgentHost::StopRemoteDebuggingServer();
}

DevToolsManagerDelegate::DevToolsManagerDelegate() {}

DevToolsManagerDelegate::~DevToolsManagerDelegate() {
}

void DevToolsManagerDelegate::DevToolsAgentHostAttached(
    content::DevToolsAgentHost* agent_host) {}

void DevToolsManagerDelegate::DevToolsAgentHostDetached(
    content::DevToolsAgentHost* agent_host) {}

base::DictionaryValue* DevToolsManagerDelegate::HandleCommand(
    content::DevToolsAgentHost* agent_host,
    base::DictionaryValue* command) {
  return nullptr;
}

}  // namespace brightray
