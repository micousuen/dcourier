20161215
1. Changed the relationship of datatrans. 
Current relationship of datatrans is:
datatrans->bdatatrans->idatatrans->iodatatrans
                     ->odatatrans-|
**Bugs:
1. Added the full detection of queue. Once the max size is limited, new push will be blocked. 

20161207
**Bugs:
1. No machanism to skip wrong destination. (Fixed. Wrong destination will be skipped)
2. ---When desitination been pointed out, the processing speed can only reach half of default speed. 
3. Delivery thread will be wake up when data pushed in. Delivery will check all sources though no wake up signal been sent to it. (Finished)

20161201
1. Finished the main body of dcourier. But it is slow, only 70,000rows/s can be processed. (include data insert and output)
**Bugs:
1. ---No machanism to process data or control data amount when output is blocked. 
2. The speed of dcourier is too slow. The use of deque and map may be the cause of this. (After changing the map in deque to struct in deque, the speed reached 16w rows/s)
3. The exit of dcourier is not safe. about 40 rows lost at the end. (Fixed. Added safe exit to delivery and out_file. Before exiting, program will check its uplevel and itself been cleared.)




