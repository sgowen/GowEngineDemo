//
//  ViewController.swift
//  GowEngineDemo
//
//  Created by Stephen Gowen on 4/22/20.
//  Copyright © 2020 Stephen Gowen. All rights reserved.
//

import UIKit

import GLKit

final class ViewController: GLKViewController
{
    deinit
    {
        engine!.releaseDeviceDependentResources()
        engine = nil
        
        EAGLContext.setCurrent(nil)
        context = nil
    }
    
    override func viewDidLoad()
    {
        super.viewDidLoad()
        
        self.context = EAGLContext(api: .openGLES3)
        
        EAGLContext.setCurrent(context)
        
        if let view = self.view as? GLKView, let context = context
        {
            view.context = context
            view.isUserInteractionEnabled = true
            view.bindDrawable()
            
            delegate = self
        }
        
        let screenBounds: CGRect = UIScreen.main.nativeBounds
        
        var size = CGSize(width: screenBounds.size.width, height: screenBounds.size.height)
        size.width = CGFloat(roundf(Float(size.width)))
        size.height = CGFloat(roundf(Float(size.height)))
        
        engine = Engine()

        engine!.createDeviceDependentResources()
        
        let width = max(size.width, size.height)
        let height = min(size.width, size.height)
        let touchWidth = UIScreen.main.bounds.size.width
        let touchHeight = UIScreen.main.bounds.size.height
        engine!.onWindowSizeChanged(screenWidth: Int32(width), screenHeight: Int32(height), cursorWidth: Int32(touchWidth), cursorHeight: Int32(touchHeight))
        
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(ViewController.willResignActive),
                                               name: UIApplication.willResignActiveNotification,
                                               object: nil)

        NotificationCenter.default.addObserver(self,
                                               selector: #selector(ViewController.didBecomeActive),
                                               name: UIApplication.didBecomeActiveNotification,
                                               object: nil)
    }
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?)
    {
        guard let touchPosition = touches.first?.location(in: view) else
        {
            return
        }
        engine!.onCursorDown(x: Float32(touchPosition.x), y: Float32(touchPosition.y), isAlt: false)
    }
    
    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?)
    {
        guard let touchPosition = touches.first?.location(in: view) else
        {
            return
        }
        engine!.onCursorDragged(x: Float32(touchPosition.x), y: Float32(touchPosition.y), isAlt: false)
    }
    
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?)
    {
        guard let touchPosition = touches.first?.location(in: view) else
        {
            return
        }
        engine!.onCursorUp(x: Float32(touchPosition.x), y: Float32(touchPosition.y), isAlt: false)
    }
    
    override var prefersHomeIndicatorAutoHidden: Bool {
        return false
    }

    override var preferredScreenEdgesDeferringSystemGestures: UIRectEdge {
        return .bottom
    }
    
    private var context: EAGLContext?
    private var engine: Engine?
    
    @objc fileprivate func willResignActive()
    {
        engine!.onPause()
    }

    @objc fileprivate func didBecomeActive()
    {
        engine!.onResume()
    }
}

// MARK: - GLKViewController Delegate
extension ViewController: GLKViewControllerDelegate
{
    func glkViewControllerUpdate(_ controller: GLKViewController)
    {
        engine!.update(deltaTime: self.timeSinceLastUpdate)
    }
}

// MARK: - GLKViewDelegate Delegate
extension ViewController
{
    override func glkView(_ view: GLKView, drawIn rect: CGRect)
    {
        engine!.render()
    }
}
