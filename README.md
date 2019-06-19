# Ledger Nano S App for Walletio
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

This is a submodule that only contains the user app according to Ledger specs.


Currently you can manually install the app with ledger nano s, we are in contact with ledger, and you can install  app through the official Leger store later.

Manual installation steps:

1. install python 2.7
2. install pip
    ```
      curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py

      python get-pip.py
    ```
3. install python env
   ```
    pip install virtualenv
    virtualenv ledger
    source ledger/bin/activate
   ```

4. install app into ledger

   download the lastest release and install app

   ```
   wget https://github.com/wallet-io/ledger-app-walletio/releases/1.0.0/walletio.tar.gz
   
   tar zxvf walletio.tar.gz ./bin

   pip install ledgerblue
   python -m ledgerblue.loadApp --appFlags 0x00 --delete --tlv --targetId 0x31100004 --fileName ./bin/app.hex --appName "WalletIo" --appVersion 0.0.1 --icon 0100000000ffffff00ffffffffffffffffffff73ce3186218463ccc7f887f08ff1cff9ffffffffffff  
   ```
   