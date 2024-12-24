#import <Foundation/Foundation.h>
#import <OSLog/OSLog.h>
#import <string>

#include "OSLogWrapper.h"

void ReadOSLogs(const char *startTimeStr)
{
  @autoreleasepool
  {
    NSError *error = nil;

    OSLogStore *store = [OSLogStore localStoreAndReturnError:&error];
    if (error)
    {
      NSLog(@"Failed to create OSLogStore: %@", error);
      return;
    }

    NSDate *startDate = nil;
    if (startTimeStr)
    {
      NSString *startTime = [NSString stringWithUTF8String:startTimeStr];
      NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
      formatter.dateFormat = @"yyyy-MM-dd HH:mm:ss";
      startDate = [formatter dateFromString:startTime];
    }
    else
    {
      startDate = [NSDate distantPast];
    }

    OSLogPosition *position = [store positionWithDate:startDate];
    NSDate *startTimestamp = [NSDate dateWithTimeIntervalSinceNow:-60 * 60 * 24 * 7];
    OSLogPosition *logPosition = [store positionWithDate:startTimestamp];
    NSPredicate *predicateTime = [NSPredicate predicateWithFormat:@"date >= %@", startTimestamp];
    NSPredicate *predicate =  [NSCompoundPredicate andPredicateWithSubpredicates:@[ predicateTime ]];

    OSLogEnumerator *enumerator =
        [store entriesEnumeratorWithOptions:0 position:logPosition predicate:predicate error:&error];

    if (error)
    {
      NSLog(@"Failed to create OSLogEnumerator: %@", error);
      return;
    }

    OSLogEntry *logEntry;

    NSLog(@"Reading logs...");
    while ((logEntry = [enumerator nextObject]))
    {
      if ([logEntry isKindOfClass:[OSLogEntryLog class]])
      {
        OSLogEntryLog *logEntrys = (OSLogEntryLog *)logEntry;
        NSLog(@"[%@] %@: %@", logEntrys.date, logEntrys.subsystem, logEntrys.composedMessage);
      }
    }
  }
}

std::string getLog()
{
  ReadOSLogs("2024-12-23 13:00:00");
  return "Logs read successfully";
}
