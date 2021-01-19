//
//  Engine.swift
//  GowEngineDemo
//
//  Created by Stephen Gowen on 4/27/20.
//  Copyright © 2020 Stephen Gowen. All rights reserved.
//

import UIKit

public class Engine: NSObject
{
    var engine: NSEngine?
    
    public override init()
    {
        engine = NSEngine()
    }
    
    public func createDeviceDependentResources()
    {
        engine!.createDeviceDependentResources()
    }
    
    public func onWindowSizeChanged(screenWidth: Int32, screenHeight: Int32, cursorWidth: Int32, cursorHeight: Int32)
    {
        engine!.onWindowSizeChanged(screenWidth, screenHeight, cursorWidth, cursorHeight)
    }
    
    public func releaseDeviceDependentResources()
    {
        engine!.releaseDeviceDependentResources()
    }
    
    public func onResume()
    {
        engine!.onResume()
    }
    
    public func onPause()
    {
        engine!.onPause()
    }
    
    public func update(deltaTime: Double)
    {
        engine!.update(deltaTime)
    }
    
    public func render()
    {
        engine!.render()
    }
    
    public func onCursorDown(x: Float32, y: Float32, isAlt: Bool)
    {
        engine!.onCursorDown(x, y, isAlt)
    }
    
    public func onCursorDragged(x: Float32, y: Float32, isAlt: Bool)
    {
        engine!.onCursorDragged(x, y, isAlt)
    }
    
    public func onCursorUp(x: Float32, y: Float32, isAlt: Bool)
    {
        engine!.onCursorUp(x, y, isAlt)
    }
}
