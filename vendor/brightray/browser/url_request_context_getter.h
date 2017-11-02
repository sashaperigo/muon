// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE-CHROMIUM file.

#ifndef BRIGHTRAY_BROWSER_URL_REQUEST_CONTEXT_GETTER_H_
#define BRIGHTRAY_BROWSER_URL_REQUEST_CONTEXT_GETTER_H_

#include "base/files/file_path.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "net/http/http_cache.h"
#include "net/http/url_security_manager.h"
#include "net/url_request/url_request_context_getter.h"

namespace base {
class MessageLoop;
}

namespace net {
class HostMappingRules;
class HostResolver;
class HttpAuthPreferences;
class NetworkDelegate;
class ProxyConfigService;
class URLRequestContextStorage;
class URLRequestJobFactory;
class URLRequestJobFactoryImpl;
}

namespace brightray {

class NetLog;

class URLRequestContextGetter : public net::URLRequestContextGetter {
 public:
  class Delegate {
   public:
    Delegate() {}
    virtual ~Delegate() {}

    virtual net::NetworkDelegate* CreateNetworkDelegate() { return NULL; }
    virtual std::string GetUserAgent();
    virtual std::unique_ptr<net::URLRequestJobFactory>
        CreateURLRequestJobFactory(
            content::ProtocolHandlerMap* protocol_handlers);
    virtual net::HttpCache::BackendFactory* CreateHttpCacheBackendFactory(
        const base::FilePath& base_path);
    virtual std::unique_ptr<net::CertVerifier> CreateCertVerifier();
    virtual net::SSLConfigService* CreateSSLConfigService();
    virtual std::vector<std::string> GetCookieableSchemes();
  };

  URLRequestContextGetter(
      Delegate* delegate,
      NetLog* net_log,
      const base::FilePath& base_path,
      bool in_memory,
      scoped_refptr<base::SingleThreadTaskRunner> io_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> file_task_runner,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector protocol_interceptors);
  virtual ~URLRequestContextGetter();

  // net::URLRequestContextGetter:
  net::URLRequestContext* GetURLRequestContext() override;
  scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner() const override;

  net::HostResolver* host_resolver();
  net::URLRequestJobFactoryImpl* job_factory() const { return job_factory_; }
  void set_job_factory(net::URLRequestJobFactoryImpl* job_factory) {
    job_factory_  = job_factory;
  }
  void NotifyContextShuttingDown();
 private:
  Delegate* delegate_;

  NetLog* net_log_;
  base::FilePath base_path_;
  bool in_memory_;
  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> file_task_runner_;

  std::string user_agent_;

  std::unique_ptr<net::ProxyConfigService> proxy_config_service_;
  std::unique_ptr<net::NetworkDelegate> network_delegate_;
  std::unique_ptr<net::URLRequestContextStorage> storage_;
  std::unique_ptr<net::URLRequestContext> url_request_context_;
  std::unique_ptr<net::HostMappingRules> host_mapping_rules_;
  std::unique_ptr<net::HttpAuthPreferences> http_auth_preferences_;
  std::unique_ptr<net::HttpNetworkSession> http_network_session_;
  content::ProtocolHandlerMap protocol_handlers_;
  content::URLRequestInterceptorScopedVector protocol_interceptors_;

  net::URLRequestJobFactoryImpl* job_factory_;  // not owned

  bool shutting_down_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestContextGetter);
};

}  // namespace brightray

#endif
