// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"


void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();

    FVector2D ViewportSize;
    if(GEngine)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }
}
