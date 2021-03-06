# Documentation

General information, compilation and installation instructions

<!-- toc -->

- [Tutorials](#tutorials)
- [Block explorers](#block-explorers)
- [Faucet](#faucet)
- [Videos](#videos)
- [Articles](#articles)
- [Original Readme](#original-readme)

<!-- tocstop -->

## Tutorials

- [Original Readme](#original-readme)- text below
- [Original HOWTO](/docs/HOWTO.md) - Step-by-step instructions for compiling and creating a simple smart contract
- [ubuntu16.04.sh](https://github.com/copperbits/TON/blob/master/docs/ubuntu16.04.sh) - Build instructions for ubuntu 16.04

## Block explorers

- [TON explorer](https://explorer.test.ton.cryptoprocessing.io) - beta explorer for test network
- [Open source TONexplorer](http://tonwatcher.com) - by @EmelyanenkoK

## Faucet

- [Faucet for TON](https://faucet.copperbits.io) - UI friendly faucet to get test Grams


## Videos

- [Telegram Open Network Tutorial](https://www.youtube.com/watch?v=J7K2nq5lf7I&feature=youtu.be) - How to use light client by [@rstormsf](https://twitter.com/rstormsf)

## Articles
- [Telegram Open Network (TON), Polkadot, Cosmos Network](https://medium.com/@buttonwallet/telegram-open-network-ton-polkadot-cosmos-network-future-of-blockchains-7b466b8f2e40) - Review by Button wallet

---
## Original Readme

_(reformated a bit, for markdown support)_

This archive is a distribution of a preliminary version of the TON Blockchain Lite Client along with the relevant portions of the TON Blockchain Library. It is not necessarily representative of the totality of the TON Blockchain code developed so far; rather it is a simplified stable version, containing only those files that are necessary for compiling the Lite Client, and sometimes outdated versions of these files sufficient for this purpose.

Use this software at your own risk; consult the DISCLAIMER for more information.

The software is licensed under GNU Lesser General Public License version 2 or later; consult LICENSE.LGPL and LGPL.v2 for more information. If you ever use any of these source files to develop your own versions of this or other software, you must attach a comment with the contents of LGPL.v2 to the beginning of each source file taken from this archive.

The software is likely to compile and work properly on most Linux systems. It should work on macOS and even Windows; however, we do not guarantee this for this preliminary version.

**Basic compilation and installation instructions**

1. Download and unpack the newest version of this archive, available at
https://test.ton.org/download
The TON Blockchain Test Network is updated quite often, so we cannot guarantee that older versions of the Lite Client will always work. Backward compatibility is not enforced at this development stage.

2. Install the newest versions of make, cmake (version 3.0.2 or later), OpenSSL (including C header files), and g++ or clang (or another C++14-compatible compiler as appropriate for your operating system).

3. Suppose that you have unpacked this archive to directory ~/lite-client, where ~ is your home directory, and that you have created an empty directory ~/liteclient-build. Then run the following in a terminal on a Linux system:

  ```bash
  cd ~/liteclient-build
  cmake ~/lite-client
  cmake --build . --target lite-client
  ```
You might also want to build some extra utilities useful for smart-contract development:

  ```bash
  cmake --build . --target fift
  ```

4. Download the newest configuration file from https://test.ton.org/ton-lite-client-test1.config.json :

  ```bash
  wget https://test.ton.org/ton-lite-client-test1.config.json
  ```

5. Run the Lite Client:

  ```bash
  ./lite-client/lite-client -C ton-lite-client-test1.config.json
  ```

  If everything was installed successfully, the Lite Client will connect to a special server (a full node for the TON Blockchain Test Network #1) and will send some queries to the server.
  If you indicate a writeable "database" directory as an extra argument to the client, it will download and save the block and the state corresponding to the newest masterchain block:

  ```bash
  ./lite-client/lite-client -C ton-lite-client-test1.config.json -D ~/ton-db-dir
  ```

  Basic help info can be obtained by typing "help" into the Lite Client. Type "quit" or press Ctrl-C to exit.

6. Now you can create new smart contracts, examine the state of existing smart contracts, send external messages to smart contracts and so on. You can also use Fift (if you have compiled it) to compile, execute, and debug your smart contracts locally.

  More details on these activities, including step-by-step instructions for creating a simple wallet smart contract (along with its source code), may be found in the HOWTO file included in this archive.

7. Some documentation on the TON Blockchain and TON Virtual Machine may be found at the download page https://test.ton.org/download . Be aware that this documentation may not be completely in sync with the version currently employed by the Test Network, because some minor implementation details are likely to be changed during the final development and testing phases.
