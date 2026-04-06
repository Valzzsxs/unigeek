import FeatureArticle from "@/components/FeatureArticle";
import GlitcheLayout from "@/layouts/GlitcheLayout";
import { getAllFeatures, getFeatureBySlug } from "@/content/features/index";
import { notFound } from "next/navigation";

export async function generateStaticParams() {
  const features = getAllFeatures();
  return features.filter((f) => f.hasDetail).map((f) => ({ slug: f.slug }));
}

const FeaturePage = ({ params }) => {
  const feature = getFeatureBySlug(params.slug);
  if (!feature) notFound();

  return (
    <GlitcheLayout>
      <FeatureArticle
        title={feature.title}
        slug={params.slug}
        category={feature.category}
        html={feature.html}
      />
    </GlitcheLayout>
  );
};

export default FeaturePage;