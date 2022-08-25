#include "MacHelpers.h"

#include <Cocoa/Cocoa.h>

namespace AK::ReaWwise::MacHelpers
{
	void makeWindowFloatingPanel(juce::Component* component)
	{
		juce::ComponentPeer* componentPeer = component->getPeer();
		componentPeer->setAlwaysOnTop(true);
		NSView* const nativeHandle = (NSView*)(componentPeer->getNativeHandle());
		NSWindow *window = [nativeHandle window];
		[window setHidesOnDeactivate:YES];
	}
}