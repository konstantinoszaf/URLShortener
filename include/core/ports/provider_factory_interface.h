#pragma once
#include <memory>
#include "core/domain/types.h"

class IProvider;
class IHttpClient;
class IEnvReader;

class IProviderFactory {
public:
    virtual ~IProviderFactory() = default;
    virtual std::shared_ptr<IProvider> createProvider(ProviderType type) = 0;
};