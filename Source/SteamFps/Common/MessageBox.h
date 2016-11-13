// Copyright (c) 2016 V1 Interactive Inc - All Rights Reserved.
#pragma once

class V1MessageBox
{
public:

    static EAppReturnType::Type Open(EAppMsgType::Type const boxType, char const* message, char const* title)
    {
        FText const messageBoxTitle = FText::FromString(title);
        FText const messageBody = FText::FromString(message);
        return V1MessageBox::Open(boxType, messageBody, messageBoxTitle);
    }

    static EAppReturnType::Type Open(EAppMsgType::Type const boxType, FString const& message, FString const& title)
    {
        auto messageBoxTitle = FText::FromString(title);
        FText const messageBody = FText::FromString(message);
        return V1MessageBox::Open(boxType, messageBody, messageBoxTitle);
    }

    static EAppReturnType::Type Open(EAppMsgType::Type const boxType, FText const& message, FText const& title)
    {
        return FMessageDialog::Open(boxType, message, &title);
    }

};