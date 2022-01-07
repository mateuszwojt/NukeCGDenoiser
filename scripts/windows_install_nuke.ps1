# download and install Nuke
Invoke-WebRequest -Uri "https://thefoundry.s3.amazonaws.com/products/nuke/releases/12.2v10/Nuke12.2v10-win-x86_64.zip" -OutFile "C:\Nuke12.2v10-win-x86_64.zip"
Expand-Archive "C:\Nuke12.2v10-win-x86_64.zip" -DestinationPath "C:\Nuke12.2v10-win-x86_64"
cd "C:\Nuke12.2v10-win-x86_64"
.\Nuke12.2v10-win-x86_64.exe /S /ACCEPT-FOUNDRY-EULA