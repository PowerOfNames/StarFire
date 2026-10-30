-- Implement the analyzeexternal command: "Off" emits <DisableAnalyzeExternal>true</DisableAnalyzeExternal>,
-- the "Disable Code Analysis for External Headers" project property (C/C++ > External Includes).
-- Premake has no API for it. Passing /analyze:external- through buildoptions instead would also switch code
-- analysis on for every build; the property only takes effect when analysis runs (editor or Run Code Analysis).
-- Headers count as external when they come from externalincludedirs.
require('vstudio')

premake.api.register {
	name = "analyzeexternal",
	scope = "config",
	kind = "string",
	allowed = {
		"On",
		"Off",
	},
}

premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, cfg)
	local calls = base(cfg)
	table.insert(calls, function(cfg)
		if cfg.analyzeexternal == "Off" then
			premake.vstudio.vc2010.element("DisableAnalyzeExternal", nil, "true")
		end
	end)
	return calls
end)
