**This is a firmware update tool for ILITEK touchscreen**

**How to build (TBU):**

**How to run (TBU):**

**How to perform a Firmware Update:**
1. put ILITEK firmware update tool "ilitek_ld" into target system, and under executable path like /usr/local/.
2. check Hex firmware is ready and accessable in target system.
3. type command as below, please read following description for each argument.


```
sudo ilitek_ld FWUpgrade <Interface> <Protocol> <i2c driver file node> <i2c addr> <Hex path>
      
      <Interface> Interface between ILTIEK Touch IC and system, should be I2C or USB or I2C-HID
        
      <Protocol>  ILITEK Touch IC command protocol, should be V3 or V6,
                  V3 for ILI2312/2315/2510/2511, V6 for ILI2316/2130/2131/2132/2322/2323/2326/2520/2521
        
      <i2c driver file node> For I2C interface only, should be /dev/ilitek_ctrl
      <i2c addr>  For I2C interface only, should be 41
      
      <Hex path>  Hex path to upgrade
```
