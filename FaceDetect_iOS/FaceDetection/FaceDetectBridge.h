//
//  FaceDetectBridge.h
//  FaceDetection
//
//  Created by Nguyen Quoc Anh on 09/09/2022.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface FaceDetectBridge : NSObject

// Declare Swift function to be called in ViewController

+ (NSString *) openCVVersionString;

+ (UIImage *) detectFace: (UIImage *) image;

@end

NS_ASSUME_NONNULL_END
