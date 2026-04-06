import GlitcheLayout from "@/layouts/GlitcheLayout";
import PageBanner from "@/components/PageBanner";
import { getAllReleases } from "@/content/releases/index";
import ReleaseList from "@/components/ReleaseList";

const ReleasesPage = () => {
  const releases = getAllReleases();

  return (
    <GlitcheLayout>
      <PageBanner pageName="Release Notes" pageLink="/release-notes" />
      <ReleaseList releases={releases} />
    </GlitcheLayout>
  );
};

export default ReleasesPage;