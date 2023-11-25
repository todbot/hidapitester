OAOA

## Sgn and bundle a commandline tool like hidapitester on MacOS

### Create both "Developer ID Application" and "Developer ID Installer" certificates in Xcode

List them with:
```sh
 security find-identity -p basic -v
 ```

### Store Apple-ID credentials in the keychain

You will be asked for a profile name. Choose something easy like "codesigntodbot"
You will be asked for an app-specfic password for your apple-id.
Create app-specific password at https://appleid.apple.com/account/manage at bottom of page

```sh
xcrun notarytool store-credentials --apple-id "tod@todbot.com" --team-id "K3Y8NR25XK"
```

### Install `pycodesign.py` from  https://github.com/txoof/codesign/

Copy the `pycodesign.py` somewhere in your PATH

### Create a blank `pycodesign.ini`

```sh
pycodesign.py -n
```

### Fill out `pycodesign.ini`

- Set `application_id` to hash of your "Developer ID Application" cert
- Set `installer_id` to hash of your "Developer ID Installer" cert
- Set `keychain-profile` to the apple-id keychain profile name (e.g. "codesigntodbot")

### Sign and bundle the app

 ```sh
  pycodesign.py pycodesign.ini
 ```

### Check signing


```sh
spctl -vv --assess ./hidapitester
spctl -vv --assess --type install ./hidapitester.pkg

```


##  Or do it by hand:

```sh
# codesign app with "Devleoper ID Application" cert
codesign --verbose=4 --force --options=runtime --timestamp  \
          --sign "DAA05419BF439B3CB8FCC9FF32720157CA65xxxx" \
          ./hidapitester

mkdir -f tmp/usr/local/bin
cp hidapitester tmp/usr/local/bin

# sign .pkg with "Developer ID Installer" cert
productbuild --identifier "com.todbot.hidapitester" \
             --sign "6B9FF39CEA9F464B78BA1847C7C0FCCD5B82xxxx" \
             --timestamp --root tmp/usr / \
             hidapitester.pkg

# notarize
xcrun notarytool submit ./hidapitester.pkg --keychain-profile "codesigntodbot" --wait

# there's also a stapling step
xcrun stapler staple hidapitester.pkg

```



## Notes:

- https://github.com/txoof/codesign/blob/main/Signing_and_Notarizing_HOWTO.md
- https://scriptingosx.com/2021/07/notarize-a-command-line-tool-with-notarytool/
- https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci/

- https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution/customizing_the_notarization_workflow
- https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution/resolving_common_notarization_issues#3087721
