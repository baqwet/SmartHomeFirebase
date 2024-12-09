# SmartHome System  

## **Mô tả dự án**  
Dự án SmartHome được thiết kế nhằm tích hợp công nghệ IoT với các vi điều khiển **ESP32** và **STM32** để xây dựng một hệ thống nhà thông minh. Hệ thống cung cấp khả năng giám sát và điều khiển từ xa thông qua Firebase và ứng dụng di động, đồng thời hỗ trợ tự động hóa các thiết bị như quạt, đèn, bơm nước và cửa ra vào.  

---

## **Chức năng chính**  
1. **ESP32 DEVKIT V1:**  
   - Gửi trạng thái cảm biến và thiết bị lên Firebase và màn hình cảm ứng HMI.  
   - Kết nối cảm biến nhiệt độ, độ ẩm (DHT11) và nhận dữ liệu từ STM32 (cảm biến khí gas MQ-2) qua UART.  
   - Nhận lệnh điều khiển từ Firebase (MIT App Inventor) để bật/tắt thiết bị như đèn, quạt, máy bơm, hoặc cửa ra vào.  
   - Quản lý cảm biến vân tay AS608 để hỗ trợ bảo mật mở cửa tự động.  

2. **STM32F103C8T6:**  
   - Thu thập dữ liệu từ cảm biến khí gas (MQ-2) và gửi dữ liệu đến ESP32 qua UART để hiển thị và cảnh báo.  
   - Điều khiển thiết bị như quạt, đèn, máy bơm và cửa ra vào theo lệnh từ ESP32.  
   - Thu thập dữ liệu cảm biến độ ẩm đất và tự động bật máy bơm khi độ ẩm thấp.  

3. **Màn hình cảm ứng (HMI):**  
   - Hiển thị thông số cảm biến (nhiệt độ, độ ẩm, khí gas).  
   - Điều khiển trực tiếp các thiết bị thông qua giao diện cảm ứng.  
