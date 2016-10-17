#include "View\Framework\WindowContext.h"

WindowContext::WindowContext(
    HINSTANCE hInstance,
    ScrollProvider* scrollProvider,
    ScaleProvider* scaleProvider,
    DeviceContextProvider* deviceContextProvider,
    ErrorHandler* errorHandler,
    RenderingContext* renderingContext,
    Logger* logger)
{
    this->hInstance = hInstance;
    this->scrollProvider = scrollProvider;
    this->scaleProvider = scaleProvider;
    this->deviceContextProvider = deviceContextProvider;
    this->renderingContext = renderingContext;
    this->errorHandler = errorHandler;
    this->logger = logger;
}

HINSTANCE WindowContext::GetInstance() const
{
    return hInstance;
}

ScrollProvider* WindowContext::GetScrollProvider() const
{
    return scrollProvider;
}

ScaleProvider* WindowContext::GetScaleProvider() const
{
    return scaleProvider;
}

DeviceContextProvider* WindowContext::GetDeviceContextProvider() const
{
    return deviceContextProvider;
}

ErrorHandler* WindowContext::GetErrorHandler() const
{
    return errorHandler;
}

RenderingContext* WindowContext::GetRenderingContext() const
{
    return renderingContext;
}

Logger* WindowContext::GetLogger() const
{
    return logger;
}

WindowContext::~WindowContext()
{
}
