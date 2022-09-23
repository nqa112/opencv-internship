//
//  QRReader.h
//  QRCodeFramework
//
//  Created by Nguyen Quoc Anh on 11/12/2021.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface QRReaderWrapper : NSObject

// Detect QR code function
+ (UIImage *) QRReader: (UIImage *) image;

@end

NS_ASSUME_NONNULL_END
