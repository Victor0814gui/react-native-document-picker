#pragma once

#include "JSValue.h"
#include "NativeModules.h"

#include "pch.h"
#include <filesystem>
#include <windows.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.System.h>
#include <windows.h>
  
using namespace winrt;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Foundation;
using namespace Windows::Storage::FileProperties;
using namespace Windows::Storage::Pickers;
using namespace Windows::System;
using namespace Windows::UI::Xaml;

using namespace winrt::Microsoft::ReactNative;


namespace RN = winrt::Microsoft::ReactNative;

namespace winrt::ReactNativeDocumentPicker
{

REACT_MODULE(ReactNativeModule, L"ReactNativeDocumentPicker")
struct ReactNativeModule
{
    // See https://microsoft.github.io/react-native-windows/docs/native-modules for details on writing native modules

    REACT_INIT(Initialize)
    void Initialize(ReactContext const &reactContext) noexcept
    {
        m_reactContext = reactContext;
    }
    
     REACT_METHOD(pickFile, L"pickFile");
        void pickFile(winrt::Microsoft::ReactNative::ReactPromise<JSValueObject> promise) noexcept
        {
            try {
                auto jsDispatcher = m_reactContext.JSDispatcher();
                m_reactContext.UIDispatcher().Post([promise, jsDispatcher] {
                    FileOpenPicker filePicker;
                    filePicker.ViewMode(PickerViewMode::Thumbnail);
                    filePicker.FileTypeFilter().ReplaceAll({ L"*" }); // Allowed File types
                    filePicker.SuggestedStartLocation(PickerLocationId::Downloads);

                    auto asyncOperation = filePicker.PickSingleFileAsync();
                    asyncOperation.Completed([jsDispatcher, promise](const IAsyncOperation<StorageFile>& asyncOp, AsyncStatus status) {
                        if (status == AsyncStatus::Completed) {
                            StorageFile text = asyncOp.GetResults();

                            auto basicPropertiesAsync = text.GetBasicPropertiesAsync();


                            basicPropertiesAsync.Completed([jsDispatcher, text, promise](
                              const IAsyncOperation<BasicProperties>& result,
                              AsyncStatus basicPropertiesStatus
                              ) {
                                  if (basicPropertiesStatus == AsyncStatus::Completed) {

                                      auto basicProperties = result.GetResults();
                                      jsDispatcher.Post([promise, basicProperties, text] {
                                          JSValueObject pickSingleObject = JSValueObject{};

                                          pickSingleObject["size"] = basicProperties.Size();
                                          pickSingleObject["path"] = winrt::to_string(text.Path());
                                          pickSingleObject["fileCopyUri"] = winrt::to_string(text.Path());
                                          pickSingleObject["type"] = winrt::to_string(text.FileType());
                                          pickSingleObject["name"] = winrt::to_string(text.Name());
                                          promise.Resolve(pickSingleObject);
                                      });
                                  }
                                  else {
                                    auto message = std::wstring(winrt::hresult_error(result.ErrorCode()).message());
                                    jsDispatcher.Post([promise, message] { promise.Reject(message.c_str()); });
                                  }
                              });
                        }
                        else {
                          auto message = std::wstring(winrt::hresult_error(asyncOp.ErrorCode()).message());
                          jsDispatcher.Post([promise, message] { promise.Reject(message.c_str()); });
                        }

                    });

                });
          }
          catch (const hresult_error& ex) {
            promise.Reject(RN::ReactError{ "Unable to make path or make_preferred for File " , winrt::to_string(ex.message()).c_str() });
           }
        }

        REACT_METHOD(pick, L"pick");
        void pick(winrt::Microsoft::ReactNative::ReactPromise<JSValueArray> promise) noexcept
        {
          try {
            auto jsDispatcher = m_reactContext.JSDispatcher();
            m_reactContext.UIDispatcher().Post([promise, jsDispatcher] {
              FileOpenPicker filePicker;
              filePicker.ViewMode(PickerViewMode::Thumbnail);
              filePicker.FileTypeFilter().ReplaceAll({ L"*" }); // Allowed File types
              filePicker.SuggestedStartLocation(PickerLocationId::Downloads);

              auto asyncOperation = filePicker.PickMultipleFilesAsync();

              asyncOperation.Completed([jsDispatcher, promise](const IAsyncOperation<IVectorView<StorageFile>>& asyncOp, AsyncStatus status) {
                if (status == AsyncStatus::Completed) {

                  IVectorView<StorageFile> files = asyncOp.GetResults();

                  if (files.Size() > 0)
                  {

                    jsDispatcher.Post([promise, files] {
                      JSValueArray resultsArray = JSValueArray{};
                      for (StorageFile const& file : files)
                      {
                      
                        resultsArray.push_back(std::move(winrt::to_string(file.Name())));
                      }
                        promise.Resolve(resultsArray);
                      });
                  }
                  else {
                    auto message = std::wstring(winrt::hresult_error(asyncOp.ErrorCode()).message());
                    jsDispatcher.Post([promise, message] { promise.Reject(message.c_str()); });
                  }
                }
                else {
                  auto message = std::wstring(winrt::hresult_error(asyncOp.ErrorCode()).message());
                  jsDispatcher.Post([promise, message] { promise.Reject(message.c_str()); });
                }

                });

              });
          }
          catch (const hresult_error& ex) {
            promise.Reject(RN::ReactError{ "Unable to make path or make_preferred for File " , winrt::to_string(ex.message()).c_str() });
          }
        }

    private:
        ReactContext m_reactContext{nullptr};
        JSValue OPTION_TYPE = "type";
        JSValue CACHE_TYPE = "cache";
        JSValue OPTION_MULTIPLE = "allowMultiSelection";
        JSValue OPTION_READ_CONTENT = "readContent";
        JSValue FIELD_URI = "uri";  
        JSValue FIELD_FILE_COPY_URI = "fileCopyUri";
        JSValue FIELD_NAME = "name";
        JSValue FIELD_TYPE = "type";
        JSValue FIELD_SIZE = "size";
        JSValue FIELD_CONTENT = "content";
};

} // namespace winrt::ReactNativeDocumentPicker
