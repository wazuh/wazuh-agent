#include "packageMac.h"
#include "brewWrapper.h"
#include "macportsWrapper.h"
#include "pkgWrapper.h"
#include "sharedDefs.h"

std::shared_ptr<IPackage> FactoryBSDPackage::create(const std::pair<PackageContext, int>& ctx)
{
    std::shared_ptr<IPackage> ret;

    if (ctx.second == BREW)
    {
        ret = std::make_shared<BSDPackageImpl>(std::make_shared<BrewWrapper>(ctx.first));
    }
    else if (ctx.second == PKG || ctx.second == RCP)
    {
        ret = std::make_shared<BSDPackageImpl>(std::make_shared<PKGWrapper>(ctx.first));
    }
    else
    {
        throw std::runtime_error {"Error creating BSD package data retriever."};
    }

    return ret;
}

std::shared_ptr<IPackage> FactoryBSDPackage::create(const std::pair<SQLiteLegacy::IStatement&, const int>& ctx)
{
    std::shared_ptr<IPackage> ret;

    if (ctx.second == MACPORTS)
    {
        ret = std::make_shared<BSDPackageImpl>(std::make_shared<MacportsWrapper>(ctx.first));
    }
    else
    {
        throw std::runtime_error {"Error creating BSD package data retriever."};
    }

    return ret;
}

BSDPackageImpl::BSDPackageImpl(const std::shared_ptr<IPackageWrapper>& packageWrapper)
    : m_packageWrapper(packageWrapper)
{
}

void BSDPackageImpl::buildPackageData(nlohmann::json& package)
{
    m_packageWrapper->name(package);
    m_packageWrapper->version(package);
    m_packageWrapper->groups(package);
    m_packageWrapper->description(package);
    m_packageWrapper->architecture(package);
    m_packageWrapper->format(package);
    m_packageWrapper->source(package);
    m_packageWrapper->location(package);
    m_packageWrapper->priority(package);
    m_packageWrapper->size(package);
    m_packageWrapper->vendor(package);
    m_packageWrapper->install_time(package);
    m_packageWrapper->multiarch(package);
}
