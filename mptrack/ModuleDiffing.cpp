/*
* ModuleDiffing.cpp
* -----------------
* Purpose: ModuleDiffing.h implementations.
*
*/

#include "stdafx.h"

#include "ModuleDiffing.h"
#include "PatternClipboard.h"
#include "../common/FileReader.h"
#include "../common/mptBufferIO.h"
#include "../common/mptCRC.h"
#include "../common/mptFileIO.h"
#include "../common/mptStringFormat.h"
#include "../common/version.h"
#include "../soundlib/mod_specifications.h"
#include "../soundlib/patternContainer.h"
#include "../soundlib/plugins/PlugInterface.h"
#include "../soundlib/Sndfile.h"
#include "../common/mptString.h"
#include  "../soundlib/tuning.h"

#if 0
// RapidJSON
#include <rapidjson/prettywriter.h>
#include <rapidjson/ostreamwrapper.h>
#endif


OPENMPT_NAMESPACE_BEGIN

namespace
{
	class DiffWriter
	{
	public:

		class NodeRaii
		{
		public:
			NodeRaii(DiffWriter& writer) :
				m_writer(writer)
			{}

			~NodeRaii()
			{
				m_writer.EndNode();
			}

			void WriteKeyValue(const char* key, const std::string& value)
			{
				m_writer.WriteKeyValueImpl(key, value);
			}

			void WriteKeyValue(const char* key, const size_t value)
			{
				m_writer.WriteKeyValueImpl(key, value);
			}

			void WriteKeyValue(const char* key, const TEMPO& tempo)
			{
				MPT_STATIC_ASSERT(TEMPO::fractFact == 10000); // Tempo formatting assumes that frac part has 4 digits.
				WriteKeyValue(key, mpt::FormatVal(tempo.ToDouble(), mpt::FormatSpec("%.4f")));
			}

			DiffWriter& m_writer;
		}; // class NodeRaii

		DiffWriter(std::ostream& ostrm) :
			m_rStrm(ostrm)
		{}

		/*[[nodiscard]]*/ NodeRaii BeginNode(const char* psz)
		{
			BeginNodeImpl(psz);
			return NodeRaii(*this);
		}

	private:
		virtual void WriteKeyValueImpl(const char* key, const std::string& val) = 0;
		virtual void WriteKeyValueImpl(const char* key, const size_t val) = 0;
		virtual void BeginNodeImpl(const char* psz) = 0;
		virtual void EndNode() = 0;

	protected:
		std::ostream& m_rStrm;
	};


	class DiffWriterText : public DiffWriter
	{
	public:
		typedef DiffWriter BaseClass;

		DiffWriterText(std::ostream& ostrm) :
			BaseClass(ostrm)
		{
			AddIncompleteStatusNote();
		}

		~DiffWriterText()
		{
			MPT_ASSERT(m_nDepth == 0);
			AddIncompleteStatusNote();
		}

	private:

		void BeginNodeImpl(const char* psz) override
		{
			if (m_nDepth == 0)
			{
				WriteSectionSeparator();
				m_rStrm << "=====================\n";
				m_rStrm << "| " << psz << '\n';
				m_rStrm << "=====================\n";
			}
			else
			{
				if (m_nDepth == 1)
					WriteItemListSeparator();
				m_rStrm << psz << '\n';
			}
				
			m_nDepth++;
			
		}

		template <class T>
		void WriteKeyValueImplT(const char* key, const T& val)
		{
			m_rStrm << key << ": " << val << "\n";
		}

		void WriteKeyValueImpl(const char* key, const std::string& val) override
		{
			WriteKeyValueImplT(key, val);
		}

		void WriteKeyValueImpl(const char* key, const size_t val) override
		{
			WriteKeyValueImplT(key, val);
		}

		void WriteItemListSeparator()
		{
			m_rStrm << "-------\n";
		}

		void WriteSectionSeparator()
		{
			m_rStrm << "\n#############################\n";
		}

		void AddIncompleteStatusNote()
		{
			m_rStrm << "---------------------------------------\n"
					   "|                NOTE                 |\n"
					   "|                                     |\n"
					   "| This text representation does not   |\n"
					   "| include all module properties.      |\n"
					   "|                                     |\n"
					   "|                                     |\n"
					   "|-------------------------------------|\n";
		}

		void EndNode() override
		{
			MPT_ASSERT(m_nDepth > 0);
			--m_nDepth;
		}

		int m_nDepth = 0;
	}; // class DiffWriterText

#if 0
	class DiffWriterRapidJson : public DiffWriter
	{
	public:
		typedef DiffWriter BaseClass;

		DiffWriterRapidJson(std::ostream& ostrm) :
			BaseClass(ostrm),
			m_ostrmWrapper(ostrm),
			m_writer(m_ostrmWrapper)
		{
			m_writer.StartObject();
		}

		~DiffWriterRapidJson()
		{
			m_writer.EndObject();
		}

	private:

		void BeginNodeImpl(const char* psz) override
		{
			m_writer.Key(psz);
			m_writer.StartObject();
		}

		void WriteKeyValueImpl(const char* key, const std::string& val) override
		{
			m_writer.Key(key);
			if (val.find('\n') != val.npos || val.find('\r') != val.npos)
			{
				// Hack: If string has newlines, make an array of lines since JSON does not support human readable newlines within values.
				auto s = mpt::String::Replace(val, "\r\n", "\n");
				std::replace(s.begin(), s.end(), '\r', '\n');
				std::vector<std::string> lines;
				size_t pos = 0;
				m_writer.StartArray();
				while (pos < s.size())
				{
					auto nextPos = s.find('\n', pos);
					if (nextPos == s.npos)
						nextPos = s.size();
					m_writer.String(s.c_str() + pos, nextPos - pos);
					pos = nextPos + 1;
				}
				m_writer.EndArray();
			}
			else
				m_writer.String(val.c_str(), val.size());
		}

		void WriteKeyValueImpl(const char* key, const size_t val) override
		{
			m_writer.Key(key);
			m_writer.Uint64(val);
		}

		void EndNode() override
		{
			m_writer.EndObject();
		}

		rapidjson::OStreamWrapper m_ostrmWrapper;
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> m_writer;

	}; // class DiffWriterRapidJson
#endif

} // unnamed namespace


void ModuleToDiffableText(const mpt::PathString& path, std::ostream& ostrm)
{
	/*
	Notes:
		-Lots of fields are missing
		-Many of the fields have natural default values (e.g. empty pattern name). The output could be made somewhat smaller (and perhaps clearer) by not printing default-valued keys.
	*/

	auto spSf = std::make_unique<CSoundFile>();
	auto& sf = *spSf;
	InputFile inputFile(path);
	FileReader reader = GetFileReader(inputFile);
	sf.Create(reader, CSoundFile::loadNoPluginInstance);

	DiffWriterText diffWriter(ostrm);
	//DiffWriterRapidJson diffWriter(ostrm);

	// General information
	{
		auto nodeGeneral = diffWriter.BeginNode("General");
		nodeGeneral.WriteKeyValue("File size", reader.GetLength());
		nodeGeneral.WriteKeyValue("Title", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, sf.GetTitle()));
		nodeGeneral.WriteKeyValue("Artist", mpt::ToCharset(mpt::CharsetUTF8, sf.m_songArtist));
		nodeGeneral.WriteKeyValue("Type", sf.GetModSpecifications().fileExtension);
		nodeGeneral.WriteKeyValue("Rows per beat", sf.m_nDefaultRowsPerBeat);
		nodeGeneral.WriteKeyValue("Rows per measure", sf.m_nDefaultRowsPerMeasure);
		nodeGeneral.WriteKeyValue("File created with", mpt::ToLocale(sf.m_dwCreatedWithVersion.ToUString()).c_str());
		nodeGeneral.WriteKeyValue("File last saved with", mpt::ToLocale(sf.m_dwLastSavedWithVersion.ToUString()).c_str());
		nodeGeneral.WriteKeyValue("Initial tempo", sf.m_nDefaultTempo);
		nodeGeneral.WriteKeyValue("Ticks per row", sf.m_nDefaultSpeed);
		nodeGeneral.WriteKeyValue("Initial global volume", sf.m_nDefaultGlobalVolume);
		nodeGeneral.WriteKeyValue("VSTi vol", sf.m_nVSTiVolume);
		nodeGeneral.WriteKeyValue("Sample vol", sf.m_nSamplePreAmp);
		nodeGeneral.WriteKeyValue("Comments", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, sf.m_songMessage));
	}

	// Sequences
	{
		const SEQUENCEINDEX nSequences = sf.Order.GetNumSequences();

		auto nodeSequences = diffWriter.BeginNode("Sequences");
		nodeSequences.WriteKeyValue("Number of sequences", int(nSequences));
		for (SEQUENCEINDEX i = 0; i < nSequences; ++i)
		{
			const auto& seq = sf.Order(i);
			auto nodeSequence = diffWriter.BeginNode(mpt::format("Sequence %1")(int(i)).c_str());
			nodeSequence.WriteKeyValue("Sequence name", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, seq.GetName()));
			nodeSequence.WriteKeyValue("Sequence restart position", seq.GetRestartPos());
			mpt::ostringstream oseqstrm;
			for (ORDERINDEX oi = 0, nCount = seq.GetLengthTailTrimmed(); oi < nCount; ++oi)
			{
				if (seq[oi] == seq.GetIgnoreIndex())
					oseqstrm << "+++ ";
				if (seq[oi] == seq.GetInvalidPatIndex())
					oseqstrm << "--- ";
				else
					oseqstrm << seq[oi] << " ";
			}
			nodeSequence.WriteKeyValue("Sequence", oseqstrm.str());
		}
	}

	// Channels
	{
		auto nodeChannels = diffWriter.BeginNode("Channels");
		nodeChannels.WriteKeyValue("Number of channels", sf.GetNumChannels());
		const auto nChnCount = sf.GetNumChannels();
		for (CHANNELINDEX nChn = 0; nChn < nChnCount; ++nChn)
		{
			auto nodeChannel = diffWriter.BeginNode(mpt::format("Channel %1")(int(nChn + 1)).c_str());
			const auto& chn = sf.ChnSettings[nChn];
			nodeChannel.WriteKeyValue("Channel name", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, chn.szName));
			nodeChannel.WriteKeyValue("Flags", chn.dwFlags.value().as_bits());
			nodeChannel.WriteKeyValue("Pan", chn.nPan);
			nodeChannel.WriteKeyValue("Volume", chn.nVolume);
			nodeChannel.WriteKeyValue("Plugin", int(chn.nMixPlugin));
		}
	} // channels

	// Patterns
	{
		auto nodePatterns = diffWriter.BeginNode("Patterns");
		const auto& patterns = sf.Patterns;
		nodePatterns.WriteKeyValue("Number of patterns", patterns.GetNumPatterns());
		for (PATTERNINDEX i = 0, nCount = patterns.GetNumPatterns(); i < nCount; ++i)
		{
			auto nodePattern = diffWriter.BeginNode(mpt::format("Pattern %1")(int(i)).c_str());
			if (!patterns.IsValidPat(i))
				nodePattern.WriteKeyValue("<empty>", "");
			else
			{
				const auto& pat = patterns[i];
				nodePattern.WriteKeyValue("Pattern name", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, pat.GetName()));
				nodePattern.WriteKeyValue("Pattern row count", pat.GetNumRows());
				const ROWINDEX nLastRow = (pat.GetNumRows() >= 1) ? pat.GetNumRows() - 1 : 0;
				const CHANNELINDEX nLastChn = (pat.GetNumChannels() >= 1) ? pat.GetNumChannels() - 1 : 0;
				const auto s = PatternClipboard::CreateClipboardString(sf, i, PatternRect(PatternCursor(0, 0), PatternCursor(nLastRow, nLastChn, PatternCursor::lastColumn)), "\n");
				nodePattern.WriteKeyValue("Pattern data", s);
			}
		}
	} // patterns

	// Samples
	{
		auto nodeSamples = diffWriter.BeginNode("Samples");
		const auto nSamples = sf.GetNumSamples();
		nodeSamples.WriteKeyValue("Number of samples", nSamples);
		for (SAMPLEINDEX i = 1; i <= nSamples; ++i)
		{
			auto nodeSample = diffWriter.BeginNode(mpt::format("Sample %1")(int(i)).c_str());
			const auto& sample = sf.GetSample(i);
			nodeSample.WriteKeyValue("Sample name", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, sf.GetSampleName(i)));
			nodeSample.WriteKeyValue("Sample size in bytes", sample.GetSampleSizeInBytes());
			const uint32 crc = mpt::crc32(reinterpret_cast<const char*>(sample.pData.pSample), reinterpret_cast<const char*>(sample.pData.pSample) + sample.GetSampleSizeInBytes());
			nodeSample.WriteKeyValue("Sample CRC", crc);
		}
	} // samples

	// Instruments
	{
		auto nodeInstruments = diffWriter.BeginNode("Instruments");
		const auto nInstr = sf.GetNumInstruments();
		nodeInstruments.WriteKeyValue("Number of instruments", nInstr);
		for (INSTRUMENTINDEX i = 1; i <= nInstr; ++i)
		{
			auto nodeInstrument = diffWriter.BeginNode(mpt::format("Instrument %1")(int(i)).c_str());
			const auto pInstr = sf.Instruments[i];
			if (!pInstr)
				continue;
			const auto& instr = *pInstr;
			nodeInstrument.WriteKeyValue("Instrument name", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, sf.GetInstrumentName(i)));
			nodeInstrument.WriteKeyValue("File name", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, instr.filename));
			nodeInstrument.WriteKeyValue("Flags", instr.dwFlags.value().as_bits());
			nodeInstrument.WriteKeyValue("Fade out", instr.nFadeOut);
			nodeInstrument.WriteKeyValue("Global volume", instr.nGlobalVol);
			nodeInstrument.WriteKeyValue("Pan", instr.nPan);
			nodeInstrument.WriteKeyValue("Volume ramp up", instr.nVolRampUp);
			nodeInstrument.WriteKeyValue("MIDI bank", instr.wMidiBank);
			nodeInstrument.WriteKeyValue("MIDI program", instr.nMidiProgram);
			nodeInstrument.WriteKeyValue("MIDI channel", instr.nMidiChannel);
			nodeInstrument.WriteKeyValue("MIDI drum key", instr.nMidiDrumKey);
			nodeInstrument.WriteKeyValue("MIDI Pitch Wheel Depth", instr.midiPWD);
			nodeInstrument.WriteKeyValue("NNA", instr.nNNA);
			nodeInstrument.WriteKeyValue("DCT", instr.nDCT);
			nodeInstrument.WriteKeyValue("DNA", instr.nDNA);
			nodeInstrument.WriteKeyValue("Random panning factor", instr.nPanSwing);
			nodeInstrument.WriteKeyValue("Random volume factor", instr.nVolSwing);
			nodeInstrument.WriteKeyValue("Filter cutoff", instr.nIFC);
			nodeInstrument.WriteKeyValue("Filter resonance", instr.nIFR);
			nodeInstrument.WriteKeyValue("Pitch/Pan separation", instr.nPPS);
			nodeInstrument.WriteKeyValue("Pitch/Pan centre", instr.nPPC);
			nodeInstrument.WriteKeyValue("Plugin", instr.nMixPlug);
			nodeInstrument.WriteKeyValue("Random cutoff factor", instr.nCutSwing);
			nodeInstrument.WriteKeyValue("Random resonance factor", instr.nResSwing);
			nodeInstrument.WriteKeyValue("Filter mode", instr.nFilterMode);
			nodeInstrument.WriteKeyValue("Plugin velocity handling", instr.nPluginVelocityHandling);
			nodeInstrument.WriteKeyValue("Plugin volume handling", instr.nPluginVolumeHandling);
			nodeInstrument.WriteKeyValue("Loop BMP", instr.pitchToTempoLock);
			nodeInstrument.WriteKeyValue("Resampling", instr.nResampling);
			/*
			TODO: 
			InstrumentEnvelope VolEnv;			// Volume envelope data
			InstrumentEnvelope PanEnv;			// Panning envelope data
			InstrumentEnvelope PitchEnv;		// Pitch / filter envelope data
			uint8 NoteMap[128];					// Note mapping, e.g. C-5 => D-5.
			SAMPLEINDEX Keyboard[128];			// Sample mapping, e.g. C-5 => Sample 1
			*/

			if (instr.pTuning)
				nodeInstrument.WriteKeyValue("Tuning", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetISO8859_1, instr.pTuning->GetName())); // Note: uses different charset than others, not sure is this correct, copied from tuning.cpp.
			if (instr.nMixPlug > 0)
			{
				const SNDMIXPLUGIN &plugin = sf.m_MixPlugins[instr.nMixPlug];
				nodeInstrument.WriteKeyValue("Instrument plugin", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, plugin.GetName()));
			}
		}
	} // instruments

	// Plugins
	if (sf.GetModSpecifications().supportsPlugins)
	{
		auto nodePlugins = diffWriter.BeginNode("Plugins");
#ifndef NO_PLUGINS
		PLUGINDEX nPlugCountByLastNonEmpty = 0;
		PLUGINDEX i = 0;
		for (const auto& plug : sf.m_MixPlugins)
		{
			++i;
			if (plug.GetLibraryName() != nullptr && *plug.GetLibraryName() != '\0')
				nPlugCountByLastNonEmpty = i;
		}
			 
		nodePlugins.WriteKeyValue("Index of last non-empty plugin", int(nPlugCountByLastNonEmpty));

		for (PLUGINDEX nPlug = 0; nPlug < nPlugCountByLastNonEmpty; ++nPlug)
		{
			auto nodePlugin = diffWriter.BeginNode(mpt::format("Plugin %1")(int(nPlug + 1)).c_str());
			const auto& plug = sf.m_MixPlugins[nPlug];
			if (plug.IsValidPlugin())
			{
				nodePlugin.WriteKeyValue("Library name", plug.GetLibraryName()); // GetLibraryName() seems to be UTF8 at the time of writing (no explicit promise in the interface, though).
				nodePlugin.WriteKeyValue("Name", mpt::ToCharset(mpt::CharsetUTF8, mpt::CharsetLocaleOrUTF8, plug.GetName()));
				const uint32 plugInfoCrc = mpt::crc32(reinterpret_cast<const char*>(&plug.Info), reinterpret_cast<const char*>(&plug.Info) + sizeof(plug.Info));
				nodePlugin.WriteKeyValue("Info crc", plugInfoCrc);
				// TODO: add remaining fields.
			}
			else
				nodePlugin.WriteKeyValue("Note", "Undefined plugin");
		}
#else // case: No plugin support
		nodePlugins.WriteKeyValue("Note", "Plugin diffing is not available because the module loader application has been build without plugin support.");
#endif // NO_PLUGINS
	} // plugins


}

OPENMPT_NAMESPACE_END
