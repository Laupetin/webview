require("premake", ">=5.0.0-beta8")

function installWebview2()
	local version = "1.0.3595.46"
	local webviewDir = "build/ms-webview2"
	local versionFile = path.join(webviewDir, "ms-webview2.txt")
	local nuspecFile = path.join(webviewDir, "Microsoft.Web.WebView2.nuspec")
	local nupkgFile = path.join(webviewDir, "microsoft.web.webview2.nupkg.zip")
	local url = "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/" .. version

	if not os.isdir(webviewDir) then
		os.mkdir(webviewDir)
	end

	local installedVersion = io.readfile(versionFile)
	if installedVersion == version and os.isfile(nuspecFile) then
		return
	end

	function progress(total, current)		
		local ratio = current / total;
		ratio = math.min(math.max(ratio, 0), 1);
		local percent = math.floor(ratio * 100);
		io.write("\rDownload progress (" .. percent .. "%/100%)")
	end

	print("Downloading Microsoft.Web.WebView2 " .. version .. "...")
	local result_str, response_code = http.download(url, nupkgFile, {
		progress = progress
	})

	io.write("\n")
	
	if result_str ~= "OK" then
		premake.error("Failed to download Microsoft.Web.WebView2")
	end

	-- local hash = string.sha1(io.readfile(nupkgFile))
	-- print(hash)

	print("Extracting Microsoft.Web.WebView2 " .. version .. "...")
	zip.extract(nupkgFile, webviewDir)

	os.remove(nupkgFile)

	io.writefile(versionFile, version)
end

-- ==================
-- Workspace
-- ==================
workspace "Webview"
    location "build"
    objdir "%{wks.location}/obj"
    symbols "On"
    systemversion "latest"
    cppdialect "C++23"
    multiprocessorcompile "on"

    configurations { 
        "Debug",
        "Release" 
    }

    platforms {
        "x86",
        "x64"
    }
    defaultplatform "x64"

    filter "platforms:x86"
        architecture "x86"
    filter {}

    filter "platforms:x64"
        architecture "x86_64"
    filter {}

    filter "configurations:Debug"
        defines "_DEBUG"
        optimize "Debug"
        symbols "On"
    filter {}

    filter "configurations:Release"
        defines "NDEBUG"
        optimize "Full"
        symbols "Off"
        fatalwarnings { "All" }
    filter {}

    defines {
        "__STDC_LIB_EXT1__",
        "__STDC_WANT_LIB_EXT1__=1",
        "_CRT_SECURE_NO_WARNINGS"
    }

    if os.host() == "windows" then
        installWebview2()
    end
    
    project("webview")
        targetdir("%{wks.location}/%{cfg.buildcfg}_%{cfg.platform}")
        location "%{wks.location}"
        kind "Utility"
        language "C++"
        files {
            "include/**/*.hpp"
        }


    local exampleFiles = os.matchfiles("examples/**.cpp")
    for i = 1, #exampleFiles do
        local exampleFile = exampleFiles[i]

	    project(path.getbasename(exampleFile))
            targetdir("%{wks.location}/%{cfg.buildcfg}_%{cfg.platform}")
            location "%{wks.location}/examples"
            kind "WindowedApp"
            language "C++"

            includedirs {
                "include"
            }
            
            files {
                exampleFile
            }

            filter { "system:windows" }
                includedirs {
                    "build/ms-webview2/build/native/include"
                }
            filter {}

		    if os.istarget("linux") then
                if os.findlib("gtk-4") ~= nil then
                    filter { "system:linux", "action:gmake" }
                        buildoptions { "`pkg-config --cflags gtk4 webkitgtk-6.0`" }
                        linkoptions { "`pkg-config --libs gtk4 webkitgtk-6.0`" }
                    filter {}
                else
                    premake.error("Must have gtk4 installed")
                end
            end
    end
